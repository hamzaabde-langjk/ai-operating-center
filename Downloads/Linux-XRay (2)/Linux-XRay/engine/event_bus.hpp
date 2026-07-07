#pragma once

#include "engine/event.hpp"
#include <functional>
#include <mutex>
#include <vector>
#include <map>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace xray::engine {

using EventHandler = std::function<void(const Event&)>;
using EventPredicate = std::function<bool(const Event&)>;

class EventBus {
public:
    EventBus();
    ~EventBus();

    EventBus(const EventBus&) = delete;
    auto operator=(const EventBus&) -> EventBus& = delete;
    EventBus(EventBus&&) = delete;
    auto operator=(EventBus&&) -> EventBus& = delete;

    void subscribe(EventType type, EventHandler handler);
    void subscribeAll(EventHandler handler);
    void unsubscribe(EventType type, const EventHandler& handler);

    void publish(Event event);
    void publishSync(const Event& event);

    void start();
    void stop();
    [[nodiscard]] auto isRunning() const -> bool { return m_running; }

    void waitUntilEmpty();

private:
    void processEvents();

    std::map<EventType, std::vector<EventHandler>> m_handlers;
    std::vector<EventHandler> m_globalHandlers;
    std::queue<Event> m_eventQueue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_worker;
    std::atomic<bool> m_running{false};
    size_t m_maxQueueSize{100000};
};

} // namespace xray::engine
