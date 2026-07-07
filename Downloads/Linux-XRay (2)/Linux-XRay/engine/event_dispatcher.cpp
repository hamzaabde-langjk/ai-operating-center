#include "engine/event_dispatcher.hpp"
#include <spdlog/spdlog.h>

namespace xray::engine {

EventDispatcher::EventDispatcher(std::shared_ptr<EventBus> bus)
    : m_bus(std::move(bus)) {}

void EventDispatcher::dispatch(Event event) {
    for (const auto& filter : m_filters) {
        if (!filter(event)) {
            spdlog::trace("Event {} filtered out", static_cast<uint32_t>(event.getType()));
            return;
        }
    }

    for (auto& transformer : m_transformers) {
        event = transformer(std::move(event));
    }

    m_bus->publish(std::move(event));
}

void EventDispatcher::dispatchBatch(std::vector<Event> events) {
    for (auto& event : events) {
        dispatch(std::move(event));
    }
}

void EventDispatcher::dispatchWithPriority(Event event, EventSeverity minSeverity) {
    if (event.getSeverity() >= minSeverity) {
        dispatch(std::move(event));
    }
}

void EventDispatcher::registerFilter(EventPredicate filter) {
    m_filters.push_back(std::move(filter));
}

void EventDispatcher::registerTransformer(std::function<Event(Event)> transformer) {
    m_transformers.push_back(std::move(transformer));
}

} // namespace xray::engine
