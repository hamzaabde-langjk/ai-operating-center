#include "engine/event_bus.hpp"
#include <spdlog/spdlog.h>

namespace xray::engine {

EventBus::EventBus() = default;

EventBus::~EventBus() {
    stop();
}

void EventBus::subscribe(EventType type, EventHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_handlers[type].push_back(std::move(handler));
    spdlog::debug("Subscribed handler for event type {}", static_cast<uint32_t>(type));
}

void EventBus::subscribeAll(EventHandler handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_globalHandlers.push_back(std::move(handler));
    spdlog::debug("Subscribed global handler");
}

void EventBus::unsubscribe(EventType type, const EventHandler& handler) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_handlers.find(type);
    if (it != m_handlers.end()) {
        auto& handlers = it->second;
        handlers.erase(
            std::remove_if(handlers.begin(), handlers.end(),
                [&handler](const auto& h) { return h.target_type() == handler.target_type(); }),
            handlers.end()
        );
    }
}

void EventBus::publish(Event event) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_eventQueue.size() >= m_maxQueueSize) {
            spdlog::warn("Event queue full, dropping event type {}", 
                        static_cast<uint32_t>(event.getType()));
            return;
        }
        m_eventQueue.push(std::move(event));
    }
    m_cv.notify_one();
}

void EventBus::publishSync(const Event& event) {
    std::vector<EventHandler> handlers;
    std::vector<EventHandler> globalHandlers;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_handlers.find(event.getType());
        if (it != m_handlers.end()) {
            handlers = it->second;
        }
        globalHandlers = m_globalHandlers;
    }

    for (const auto& handler : handlers) {
        try {
            handler(event);
        } catch (const std::exception& e) {
            spdlog::error("Event handler error: {}", e.what());
        }
    }
    for (const auto& handler : globalHandlers) {
        try {
            handler(event);
        } catch (const std::exception& e) {
            spdlog::error("Global handler error: {}", e.what());
        }
    }
}

void EventBus::start() {
    if (m_running.exchange(true)) {
        return;
    }
    m_worker = std::thread(&EventBus::processEvents, this);
    spdlog::info("EventBus started");
}

void EventBus::stop() {
    if (!m_running.exchange(false)) {
        return;
    }
    m_cv.notify_all();
    if (m_worker.joinable()) {
        m_worker.join();
    }
    spdlog::info("EventBus stopped");
}

void EventBus::waitUntilEmpty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return m_eventQueue.empty(); });
}

void EventBus::processEvents() {
    while (m_running) {
        std::vector<Event> events;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return !m_eventQueue.empty() || !m_running; });

            if (!m_running) break;

            while (!m_eventQueue.empty() && events.size() < 1000) {
                events.push_back(std::move(m_eventQueue.front()));
                m_eventQueue.pop();
            }
        }

        for (const auto& event : events) {
            publishSync(event);
        }
    }
}

} // namespace xray::engine
