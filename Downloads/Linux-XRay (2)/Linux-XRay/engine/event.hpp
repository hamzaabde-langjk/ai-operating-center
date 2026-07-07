#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace xray::engine {

enum class EventType : uint32_t {
    Unknown = 0,
    ProcessExec,
    ProcessFork,
    ProcessExit,
    ProcessClone,
    MemoryMap,
    MemoryUnmap,
    PageFault,
    FileOpen,
    FileRead,
    FileWrite,
    FileRename,
    FileUnlink,
    SocketCreate,
    SocketConnect,
    SocketAccept,
    SocketSend,
    SocketReceive,
    Signal,
    SchedSwitch,
    CpuUsage,
    MemoryUsage,
    BlockIo,
    UsbEvent,
    ModuleLoad,
    CgroupEvent,
    NamespaceEvent,
    ContainerEvent,
    SecurityAlert,
    AnomalyDetected,
    UserDefined
};

enum class EventSeverity : uint8_t {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

struct ProcessInfo {
    uint32_t pid;
    uint32_t ppid;
    uint32_t tid;
    uint32_t uid;
    uint32_t gid;
    std::string comm;
    std::string exePath;
    std::vector<std::string> args;
    std::unordered_map<std::string, std::string> env;
};

struct FileInfo {
    std::string path;
    std::string operation;
    int64_t size;
    int flags;
    int mode;
};

struct NetworkInfo {
    std::string protocol;
    std::string localAddr;
    uint16_t localPort;
    std::string remoteAddr;
    uint16_t remotePort;
    uint64_t bytesTransferred;
    std::string domain;
};

struct MemoryInfo {
    uint64_t address;
    uint64_t size;
    int prot;
    int flags;
    std::string backingFile;
};

struct SecurityInfo {
    std::string alertType;
    std::string description;
    double confidence;
    std::vector<std::string> indicators;
};

using EventData = std::variant<
    ProcessInfo,
    FileInfo,
    NetworkInfo,
    MemoryInfo,
    SecurityInfo,
    std::unordered_map<std::string, std::string>
>;

class Event {
public:
    Event();
    explicit Event(EventType type);
    Event(EventType type, EventData data, EventSeverity severity = EventSeverity::Info);

    [[nodiscard]] auto getId() const -> uint64_t { return m_id; }
    [[nodiscard]] auto getType() const -> EventType { return m_type; }
    [[nodiscard]] auto getSeverity() const -> EventSeverity { return m_severity; }
    [[nodiscard]] auto getTimestamp() const -> std::chrono::nanoseconds { return m_timestamp; }
    [[nodiscard]] auto getData() const -> const EventData& { return m_data; }
    [[nodiscard]] auto getSource() const -> const std::string& { return m_source; }
    [[nodiscard]] auto getTags() const -> const std::vector<std::string>& { return m_tags; }

    void setSeverity(EventSeverity severity) { m_severity = severity; }
    void setSource(std::string source) { m_source = std::move(source); }
    void addTag(std::string tag) { m_tags.push_back(std::move(tag)); }
    void setData(EventData data) { m_data = std::move(data); }

    [[nodiscard]] auto toString() const -> std::string;
    [[nodiscard]] auto toJson() const -> std::string;

    static auto fromJson(const std::string& json) -> std::optional<Event>;

private:
    uint64_t m_id{0};
    EventType m_type{EventType::Unknown};
    EventSeverity m_severity{EventSeverity::Info};
    std::chrono::nanoseconds m_timestamp{0};
    EventData m_data;
    std::string m_source;
    std::vector<std::string> m_tags;

    static std::atomic<uint64_t> s_nextId;
};

} // namespace xray::engine
