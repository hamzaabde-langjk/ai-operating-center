#pragma once

#include "engine/event.hpp"
#include <functional>
#include <vector>

namespace xray::engine {

class EventFilter {
public:
    using Predicate = std::function<bool(const Event&)>;

    static auto byType(EventType type) -> Predicate;
    static auto byTypes(const std::vector<EventType>& types) -> Predicate;
    static auto bySeverity(EventSeverity minSeverity) -> Predicate;
    static auto bySource(const std::string& source) -> Predicate;
    static auto byTimeRange(std::chrono::nanoseconds start, 
                           std::chrono::nanoseconds end) -> Predicate;
    static auto byTag(const std::string& tag) -> Predicate;
    static auto compositeAnd(const std::vector<Predicate>& predicates) -> Predicate;
    static auto compositeOr(const std::vector<Predicate>& predicates) -> Predicate;
    static auto compositeNot(Predicate predicate) -> Predicate;
};

} // namespace xray::engine
