#pragma once

#include "engine/event.hpp"
#include <list>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace xray::engine {

template<typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity);

    [[nodiscard]] auto get(const Key& key) -> std::optional<Value>;
    void put(const Key& key, Value value);
    void invalidate(const Key& key);
    void clear();
    [[nodiscard]] auto size() const -> size_t;
    [[nodiscard]] auto capacity() const -> size_t;

private:
    size_t m_capacity;
    std::list<std::pair<Key, Value>> m_items;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> m_map;
    mutable std::mutex m_mutex;
};

class EventCache {
public:
    explicit EventCache(size_t maxEvents = 10000);

    void cacheEvent(Event event);
    [[nodiscard]] auto getRecentEvents(size_t count) const -> std::vector<Event>;
    [[nodiscard]] auto getEventsByType(EventType type) const -> std::vector<Event>;
    void clear();

private:
    LRUCache<uint64_t, Event> m_eventCache;
    std::multimap<EventType, uint64_t> m_typeIndex;
    mutable std::mutex m_indexMutex;
};

} // namespace xray::engine
