#include "engine/event_filter.hpp"

namespace xray::engine {

auto EventFilter::byType(EventType type) -> Predicate {
    return [type](const Event& e) { return e.getType() == type; };
}

auto EventFilter::byTypes(const std::vector<EventType>& types) -> Predicate {
    return [types](const Event& e) {
        return std::find(types.begin(), types.end(), e.getType()) != types.end();
    };
}

auto EventFilter::bySeverity(EventSeverity minSeverity) -> Predicate {
    return [minSeverity](const Event& e) {
        return e.getSeverity() >= minSeverity;
    };
}

auto EventFilter::bySource(const std::string& source) -> Predicate {
    return [source](const Event& e) { return e.getSource() == source; };
}

auto EventFilter::byTimeRange(std::chrono::nanoseconds start, 
                              std::chrono::nanoseconds end) -> Predicate {
    return [start, end](const Event& e) {
        auto ts = e.getTimestamp();
        return ts >= start && ts <= end;
    };
}

auto EventFilter::byTag(const std::string& tag) -> Predicate {
    return [tag](const Event& e) {
        const auto& tags = e.getTags();
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    };
}

auto EventFilter::compositeAnd(const std::vector<Predicate>& predicates) -> Predicate {
    return [predicates](const Event& e) {
        return std::all_of(predicates.begin(), predicates.end(),
                          [&e](const auto& p) { return p(e); });
    };
}

auto EventFilter::compositeOr(const std::vector<Predicate>& predicates) -> Predicate {
    return [predicates](const Event& e) {
        return std::any_of(predicates.begin(), predicates.end(),
                          [&e](const auto& p) { return p(e); });
    };
}

auto EventFilter::compositeNot(Predicate predicate) -> Predicate {
    return [predicate](const Event& e) { return !predicate(e); };
}

} // namespace xray::engine
