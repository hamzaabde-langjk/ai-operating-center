#pragma once

#include "engine/event.hpp"
#include "engine/event_bus.hpp"
#include <vector>
#include <memory>

namespace xray::engine {

class EventDispatcher {
public:
    explicit EventDispatcher(std::shared_ptr<EventBus> bus);

    void dispatch(Event event);
    void dispatchBatch(std::vector<Event> events);
    void dispatchWithPriority(Event event, EventSeverity minSeverity);

    void registerFilter(EventPredicate filter);
    void registerTransformer(std::function<Event(Event)> transformer);

private:
    std::shared_ptr<EventBus> m_bus;
    std::vector<EventPredicate> m_filters;
    std::vector<std::function<Event(Event)>> m_transformers;
};

} // namespace xray::engine
