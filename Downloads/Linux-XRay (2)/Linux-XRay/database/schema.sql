-- X-Ray Database Schema
-- Run this to initialize or migrate the database

-- Events table: stores all system events
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

-- Snapshots: system state captures
CREATE TABLE IF NOT EXISTS snapshots (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    timestamp INTEGER NOT NULL,
    data BLOB,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Replay sessions: recorded event streams
CREATE TABLE IF NOT EXISTS replay_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    start_time INTEGER NOT NULL,
    end_time INTEGER,
    event_data BLOB,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Bookmarks: user-marked points of interest
CREATE TABLE IF NOT EXISTS bookmarks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    timestamp INTEGER NOT NULL,
    event_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Statistics: time-series metrics
CREATE TABLE IF NOT EXISTS statistics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    value REAL NOT NULL,
    timestamp INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_stats_name ON statistics(name);

-- AI Analysis: ML-generated insights
CREATE TABLE IF NOT EXISTS ai_analysis (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    event_id INTEGER NOT NULL,
    analysis TEXT NOT NULL,
    confidence REAL NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_ai_event ON ai_analysis(event_id);
