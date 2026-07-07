#include "engine/event.hpp"
#include <spdlog/spdlog.h>

namespace xray::engine {

std::atomic<uint64_t> Event::s_nextId{1};

Event::Event() : m_id(s_nextId++), m_timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::steady_clock::now().time_since_epoch())) {}

Event::Event(EventType type) 
    : m_id(s_nextId++), m_type(type),
      m_timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now().time_since_epoch())) {}

Event::Event(EventType type, EventData data, EventSeverity severity)
    : m_id(s_nextId++), m_type(type), m_severity(severity),
      m_timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now().time_since_epoch())),
      m_data(std::move(data)) {}

auto Event::toString() const -> std::string {
    std::ostringstream oss;
    oss << "Event[" << m_id << "] type=" << static_cast<uint32_t>(m_type)
        << " severity=" << static_cast<uint32_t>(m_severity)
        << " ts=" << m_timestamp.count();
    return oss.str();
}

auto Event::toJson() const -> std::string {
    std::ostringstream oss;
    oss << "{";
    oss << ""id":" << m_id << ",";
    oss << ""type":" << static_cast<uint32_t>(m_type) << ",";
    oss << ""severity":" << static_cast<uint32_t>(m_severity) << ",";
    oss << ""timestamp":" << m_timestamp.count() << ",";
    oss << ""source":"" << m_source << "",";
    oss << ""tags":[";
    for (size_t i = 0; i < m_tags.size(); ++i) {
        if (i > 0) oss << ",";
        oss << """ << m_tags[i] << """;
    }
    oss << "]";
    oss << "}";
    return oss.str();
}

auto Event::fromJson(const std::string& json) -> std::optional<Event> {
    // Simplified JSON parsing - would use nlohmann/json in production
    try {
        Event event;
        // Parse id
        size_t idPos = json.find(""id":");
        if (idPos != std::string::npos) {
            event.m_id = std::stoull(json.substr(idPos + 5));
        }
        // Parse type
        size_t typePos = json.find(""type":");
        if (typePos != std::string::npos) {
            event.m_type = static_cast<EventType>(std::stoul(json.substr(typePos + 7)));
        }
        return event;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace xray::engine
