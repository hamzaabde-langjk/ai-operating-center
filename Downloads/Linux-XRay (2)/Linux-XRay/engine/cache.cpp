#include "engine/cache.hpp"

namespace xray::engine {

template<typename Key, typename Value>
LRUCache<Key, Value>::LRUCache(size_t capacity) : m_capacity(capacity) {}

template<typename Key, typename Value>
auto LRUCache<Key, Value>::get(const Key& key) -> std::optional<Value> {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_map.find(key);
    if (it == m_map.end()) {
        return std::nullopt;
    }
    m_items.splice(m_items.begin(), m_items, it->second);
    return it->second->second;
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::put(const Key& key, Value value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_map.find(key);
    if (it != m_map.end()) {
        m_items.erase(it->second);
        m_map.erase(it);
    }
    m_items.emplace_front(key, std::move(value));
    m_map[key] = m_items.begin();
    if (m_items.size() > m_capacity) {
        auto last = m_items.end();
        --last;
        m_map.erase(last->first);
        m_items.pop_back();
    }
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::invalidate(const Key& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_map.find(key);
    if (it != m_map.end()) {
        m_items.erase(it->second);
        m_map.erase(it);
    }
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_items.clear();
    m_map.clear();
}

template<typename Key, typename Value>
auto LRUCache<Key, Value>::size() const -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_items.size();
}

template<typename Key, typename Value>
auto LRUCache<Key, Value>::capacity() const -> size_t {
    return m_capacity;
}

EventCache::EventCache(size_t maxEvents) : m_eventCache(maxEvents) {}

void EventCache::cacheEvent(Event event) {
    uint64_t id = event.getId();
    EventType type = event.getType();
    m_eventCache.put(id, event);
    {
        std::lock_guard<std::mutex> lock(m_indexMutex);
        m_typeIndex.emplace(type, id);
    }
}

auto EventCache::getRecentEvents(size_t count) const -> std::vector<Event> {
    // Simplified implementation
    return {};
}

auto EventCache::getEventsByType(EventType type) const -> std::vector<Event> {
    std::lock_guard<std::mutex> lock(m_indexMutex);
    std::vector<Event> result;
    auto range = m_typeIndex.equal_range(type);
    for (auto it = range.first; it != range.second; ++it) {
        auto event = m_eventCache.get(it->second);
        if (event) {
            result.push_back(*event);
        }
    }
    return result;
}

void EventCache::clear() {
    m_eventCache.clear();
    {
        std::lock_guard<std::mutex> lock(m_indexMutex);
        m_typeIndex.clear();
    }
}

} // namespace xray::engine
