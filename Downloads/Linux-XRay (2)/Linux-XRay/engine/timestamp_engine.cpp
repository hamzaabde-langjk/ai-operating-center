#include "engine/timestamp_engine.hpp"

namespace xray::engine {

TimestampEngine::TimestampEngine()
    : m_startTime(std::chrono::steady_clock::now()) {}

auto TimestampEngine::now() const -> std::chrono::nanoseconds {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - m_startTime);
}

void TimestampEngine::recordEvent(Event event) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.emplace(event.getTimestamp(), std::move(event));
}

auto TimestampEngine::getEventsInRange(
    std::chrono::nanoseconds start,
    std::chrono::nanoseconds end) const -> std::vector<Event> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Event> result;
    auto itStart = m_events.lower_bound(start);
    auto itEnd = m_events.upper_bound(end);
    for (auto it = itStart; it != itEnd; ++it) {
        result.push_back(it->second);
    }
    return result;
}

auto TimestampEngine::getEventsBefore(
    std::chrono::nanoseconds timestamp) const -> std::vector<Event> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Event> result;
    auto it = m_events.begin();
    while (it != m_events.end() && it->first < timestamp) {
        result.push_back(it->second);
        ++it;
    }
    return result;
}

auto TimestampEngine::getEventsAfter(
    std::chrono::nanoseconds timestamp) const -> std::vector<Event> {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Event> result;
    auto it = m_events.upper_bound(timestamp);
    while (it != m_events.end()) {
        result.push_back(it->second);
        ++it;
    }
    return result;
}

void TimestampEngine::clearBefore(std::chrono::nanoseconds timestamp) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_events.begin();
    while (it != m_events.end() && it->first < timestamp) {
        it = m_events.erase(it);
    }
}

void TimestampEngine::clearAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.clear();
}

auto TimestampEngine::getEventCount() const -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events.size();
}

} // namespace xray::engine
