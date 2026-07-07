#pragma once

#include "engine/event.hpp"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>

namespace xray::database {

struct Snapshot {
    uint64_t id;
    std::chrono::system_clock::time_point timestamp;
    std::string name;
    std::string description;
    std::vector<uint8_t> data;
};

struct ReplaySession {
    uint64_t id;
    std::string name;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::vector<uint8_t> eventData;
};

struct Bookmark {
    uint64_t id;
    std::string name;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    uint64_t eventId;
};

class Database {
public:
    explicit Database(const std::string& path = "xray.db");
    ~Database();

    Database(const Database&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    Database(Database&&) = delete;
    auto operator=(Database&&) -> Database& = delete;

    bool initialize();
    bool isInitialized() const { return m_initialized; }

    // Event storage
    bool storeEvent(const engine::Event& event);
    bool storeEvents(const std::vector<engine::Event>& events);
    [[nodiscard]] auto getEvents(size_t limit = 1000, size_t offset = 0) const -> std::vector<engine::Event>;
    [[nodiscard]] auto getEventsByType(engine::EventType type, size_t limit = 1000) const -> std::vector<engine::Event>;
    [[nodiscard]] auto getEventsByTimeRange(
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end) const -> std::vector<engine::Event>;
    [[nodiscard]] auto getEventCount() const -> size_t;

    // Snapshots
    bool createSnapshot(const Snapshot& snapshot);
    [[nodiscard]] auto getSnapshots() const -> std::vector<Snapshot>;
    [[nodiscard]] auto getSnapshot(uint64_t id) const -> std::optional<Snapshot>;
    bool deleteSnapshot(uint64_t id);

    // Replay sessions
    bool createReplaySession(const ReplaySession& session);
    [[nodiscard]] auto getReplaySessions() const -> std::vector<ReplaySession>;
    [[nodiscard]] auto getReplaySession(uint64_t id) const -> std::optional<ReplaySession>;

    // Bookmarks
    bool addBookmark(const Bookmark& bookmark);
    [[nodiscard]] auto getBookmarks() const -> std::vector<Bookmark>;
    bool deleteBookmark(uint64_t id);

    // Statistics
    bool storeStatistic(const std::string& name, double value, std::chrono::system_clock::time_point timestamp);
    [[nodiscard]] auto getStatistics(const std::string& name, size_t limit = 1000) const -> std::vector<std::pair<std::chrono::system_clock::time_point, double>>;

    // AI Analysis
    bool storeAiAnalysis(uint64_t eventId, const std::string& analysis, double confidence);
    [[nodiscard]] auto getAiAnalysis(uint64_t eventId) const -> std::optional<std::string>;

    // Maintenance
    bool vacuum();
    bool backup(const std::string& path);
    [[nodiscard]] auto getSize() const -> size_t;

    // Transactions
    bool beginTransaction();
    bool commit();
    bool rollback();

private:
    sqlite3* m_db{nullptr};
    std::string m_path;
    bool m_initialized{false};
    mutable std::mutex m_mutex;

    bool executeScript(const std::string& script);
    [[nodiscard]] auto prepareStatement(const std::string& sql) const -> sqlite3_stmt*;
    static void eventToSqlite(sqlite3_stmt* stmt, const engine::Event& event);
    static auto sqliteToEvent(sqlite3_stmt* stmt) -> engine::Event;
};

} // namespace xray::database
