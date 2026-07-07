#pragma once

#include "engine/event.hpp"
#include <chrono>
#include <vector>
#include <map>
#include <mutex>

namespace xray::engine {

class TimestampEngine {
public:
    TimestampEngine();

    [[nodiscard]] auto now() const -> std::chrono::nanoseconds;
    void recordEvent(Event event);

    [[nodiscard]] auto getEventsInRange(
        std::chrono::nanoseconds start,
        std::chrono::nanoseconds end) const -> std::vector<Event>;

    [[nodiscard]] auto getEventsBefore(
        std::chrono::nanoseconds timestamp) const -> std::vector<Event>;

    [[nodiscard]] auto getEventsAfter(
        std::chrono::nanoseconds timestamp) const -> std::vector<Event>;

    void clearBefore(std::chrono::nanoseconds timestamp);
    void clearAll();

    [[nodiscard]] auto getEventCount() const -> size_t;

private:
    mutable std::mutex m_mutex;
    std::multimap<std::chrono::nanoseconds, Event> m_events;
    std::chrono::steady_clock::time_point m_startTime;
};

} // namespace xray::engine
