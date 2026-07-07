#include "database/database.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>

namespace xray::database {

Database::Database(const std::string& path) : m_path(path) {}

Database::~Database() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}

bool Database::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);

    int rc = sqlite3_open(m_path.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        spdlog::error("Cannot open database: {}", sqlite3_errmsg(m_db));
        return false;
    }

    // Enable WAL mode for better concurrency
    sqlite3_exec(m_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);

    // Create schema
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_id INTEGER NOT NULL,
            event_type INTEGER NOT NULL,
            severity INTEGER NOT NULL,
            timestamp INTEGER NOT NULL,
            source TEXT,
            data BLOB,
            tags TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_events_type ON events(event_type);
        CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp);
        CREATE INDEX IF NOT EXISTS idx_events_source ON events(source);

        CREATE TABLE IF NOT EXISTS snapshots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            timestamp INTEGER NOT NULL,
            data BLOB,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS replay_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            start_time INTEGER NOT NULL,
            end_time INTEGER,
            event_data BLOB,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS bookmarks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT,
            timestamp INTEGER NOT NULL,
            event_id INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS statistics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            value REAL NOT NULL,
            timestamp INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_stats_name ON statistics(name);

        CREATE TABLE IF NOT EXISTS ai_analysis (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_id INTEGER NOT NULL,
            analysis TEXT NOT NULL,
            confidence REAL NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_ai_event ON ai_analysis(event_id);
    )";

    rc = sqlite3_exec(m_db, schema, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Failed to create schema: {}", sqlite3_errmsg(m_db));
        return false;
    }

    m_initialized = true;
    spdlog::info("Database initialized: {}", m_path);
    return true;
}

bool Database::storeEvent(const engine::Event& event) {
    std::lock_guard<std::mutex> lock(m_mutex);

    const char* sql = "INSERT INTO events (event_id, event_type, severity, timestamp, source, data, tags) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        spdlog::error("Prepare failed: {}", sqlite3_errmsg(m_db));
        return false;
    }

    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(event.getId()));
    sqlite3_bind_int(stmt, 2, static_cast<int>(event.getType()));
    sqlite3_bind_int(stmt, 3, static_cast<int>(event.getSeverity()));
    sqlite3_bind_int64(stmt, 4, event.getTimestamp().count());
    sqlite3_bind_text(stmt, 5, event.getSource().c_str(), -1, SQLITE_STATIC);

    // Serialize data as JSON
    std::string jsonData = event.toJson();
    sqlite3_bind_blob(stmt, 6, jsonData.data(), static_cast<int>(jsonData.size()), SQLITE_STATIC);

    std::string tags;
    for (const auto& tag : event.getTags()) {
        if (!tags.empty()) tags += ",";
        tags += tag;
    }
    sqlite3_bind_text(stmt, 7, tags.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        spdlog::error("Insert failed: {}", sqlite3_errmsg(m_db));
        return false;
    }
    return true;
}

bool Database::storeEvents(const std::vector<engine::Event>& events) {
    beginTransaction();
    for (const auto& event : events) {
        if (!storeEvent(event)) {
            rollback();
            return false;
        }
    }
    return commit();
}

auto Database::getEvents(size_t limit, size_t offset) const -> std::vector<engine::Event> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<engine::Event> result;

    const char* sql = "SELECT event_id, event_type, severity, timestamp, source, data, tags "
                      "FROM events ORDER BY timestamp DESC LIMIT ? OFFSET ?";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(limit));
    sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(offset));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        result.push_back(sqliteToEvent(stmt));
    }
    sqlite3_finalize(stmt);
    return result;
}

auto Database::getEventsByType(engine::EventType type, size_t limit) const -> std::vector<engine::Event> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<engine::Event> result;

    const char* sql = "SELECT event_id, event_type, severity, timestamp, source, data, tags "
                      "FROM events WHERE event_type = ? ORDER BY timestamp DESC LIMIT ?";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, static_cast<int>(type));
    sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(limit));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        result.push_back(sqliteToEvent(stmt));
    }
    sqlite3_finalize(stmt);
    return result;
}

auto Database::getEventCount() const -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, "SELECT COUNT(*) FROM events", -1, &stmt, nullptr);
    size_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return count;
}

bool Database::createSnapshot(const Snapshot& snapshot) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const char* sql = "INSERT INTO snapshots (name, description, timestamp, data) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, snapshot.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, snapshot.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, std::chrono::duration_cast<std::chrono::nanoseconds>(
        snapshot.timestamp.time_since_epoch()).count());
    sqlite3_bind_blob(stmt, 4, snapshot.data.data(), static_cast<int>(snapshot.data.size()), SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

auto Database::getSnapshots() const -> std::vector<Snapshot> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Snapshot> result;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, "SELECT id, name, description, timestamp, data FROM snapshots ORDER BY timestamp DESC", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Snapshot snap;
        snap.id = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        snap.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        snap.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        auto ts = sqlite3_column_int64(stmt, 3);
        snap.timestamp = std::chrono::system_clock::time_point(std::chrono::nanoseconds(ts));
        const void* data = sqlite3_column_blob(stmt, 4);
        int size = sqlite3_column_bytes(stmt, 4);
        if (data && size > 0) {
            snap.data.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);
        }
        result.push_back(std::move(snap));
    }
    sqlite3_finalize(stmt);
    return result;
}

bool Database::addBookmark(const Bookmark& bookmark) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const char* sql = "INSERT INTO bookmarks (name, description, timestamp, event_id) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, bookmark.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, bookmark.description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, std::chrono::duration_cast<std::chrono::nanoseconds>(
        bookmark.timestamp.time_since_epoch()).count());
    sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(bookmark.eventId));
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

auto Database::getBookmarks() const -> std::vector<Bookmark> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Bookmark> result;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, "SELECT id, name, description, timestamp, event_id FROM bookmarks ORDER BY timestamp DESC", -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Bookmark bm;
        bm.id = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
        bm.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bm.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        auto ts = sqlite3_column_int64(stmt, 3);
        bm.timestamp = std::chrono::system_clock::time_point(std::chrono::nanoseconds(ts));
        bm.eventId = static_cast<uint64_t>(sqlite3_column_int64(stmt, 4));
        result.push_back(std::move(bm));
    }
    sqlite3_finalize(stmt);
    return result;
}

bool Database::storeStatistic(const std::string& name, double value, std::chrono::system_clock::time_point timestamp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const char* sql = "INSERT INTO statistics (name, value, timestamp) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, value);
    sqlite3_bind_int64(stmt, 3, std::chrono::duration_cast<std::chrono::nanoseconds>(
        timestamp.time_since_epoch()).count());
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

auto Database::getStatistics(const std::string& name, size_t limit) const -> std::vector<std::pair<std::chrono::system_clock::time_point, double>> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::pair<std::chrono::system_clock::time_point, double>> result;
    const char* sql = "SELECT timestamp, value FROM statistics WHERE name = ? ORDER BY timestamp DESC LIMIT ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(limit));
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto ts = sqlite3_column_int64(stmt, 0);
        auto time = std::chrono::system_clock::time_point(std::chrono::nanoseconds(ts));
        double value = sqlite3_column_double(stmt, 1);
        result.emplace_back(time, value);
    }
    sqlite3_finalize(stmt);
    return result;
}

bool Database::storeAiAnalysis(uint64_t eventId, const std::string& analysis, double confidence) {
    std::lock_guard<std::mutex> lock(m_mutex);
    const char* sql = "INSERT INTO ai_analysis (event_id, analysis, confidence) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(eventId));
    sqlite3_bind_text(stmt, 2, analysis.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, confidence);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

auto Database::getAiAnalysis(uint64_t eventId) const -> std::optional<std::string> {
    std::lock_guard<std::mutex> lock(m_mutex);
    const char* sql = "SELECT analysis FROM ai_analysis WHERE event_id = ? ORDER BY created_at DESC LIMIT 1";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(eventId));
    std::optional<std::string> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return result;
}

bool Database::vacuum() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return sqlite3_exec(m_db, "VACUUM;", nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool Database::backup(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    sqlite3* backupDb;
    int rc = sqlite3_open(path.c_str(), &backupDb);
    if (rc != SQLITE_OK) return false;

    sqlite3_backup* backup = sqlite3_backup_init(backupDb, "main", m_db, "main");
    if (!backup) {
        sqlite3_close(backupDb);
        return false;
    }

    sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);
    sqlite3_close(backupDb);
    return true;
}

auto Database::getSize() const -> size_t {
    std::ifstream file(m_path, std::ios::binary | std::ios::ate);
    return file.is_open() ? static_cast<size_t>(file.tellg()) : 0;
}

bool Database::beginTransaction() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool Database::commit() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool Database::rollback() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr) == SQLITE_OK;
}

auto Database::sqliteToEvent(sqlite3_stmt* stmt) -> engine::Event {
    uint64_t eventId = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
    auto type = static_cast<engine::EventType>(sqlite3_column_int(stmt, 1));
    auto severity = static_cast<engine::EventSeverity>(sqlite3_column_int(stmt, 2));
    auto timestamp = std::chrono::nanoseconds(sqlite3_column_int64(stmt, 3));

    engine::Event event(type);
    event.setSeverity(severity);
    // Note: In a full implementation, we'd reconstruct the EventData from the JSON blob

    const char* source = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    if (source) event.setSource(source);

    return event;
}

} // namespace xray::database
