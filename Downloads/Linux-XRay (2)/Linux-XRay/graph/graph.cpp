#include "graph/graph.hpp"
#include <spdlog/spdlog.h>
#include <queue>

namespace xray::graph {

Graph::Graph() = default;

auto Graph::addNode(NodeType type, std::string name) -> std::shared_ptr<Node> {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    uint64_t id = m_nextId++;
    auto node = std::make_shared<Node>(id, type, std::move(name));
    m_nodes[id] = node;
    m_typeIndex[type].insert(id);
    spdlog::trace("Added node {} of type {}", id, static_cast<uint32_t>(type));
    return node;
}

void Graph::removeNode(uint64_t id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_nodes.find(id);
    if (it == m_nodes.end()) return;

    auto type = it->second->getType();
    m_typeIndex[type].erase(id);

    // Remove associated edges
    auto adjIt = m_adjacencyList.find(id);
    if (adjIt != m_adjacencyList.end()) {
        for (uint64_t edgeId : adjIt->second) {
            m_edges.erase(edgeId);
        }
        m_adjacencyList.erase(adjIt);
    }

    m_nodes.erase(it);
    spdlog::trace("Removed node {}", id);
}

auto Graph::getNode(uint64_t id) const -> std::shared_ptr<Node> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_nodes.find(id);
    if (it != m_nodes.end()) {
        return it->second;
    }
    return nullptr;
}

auto Graph::getNodesByType(NodeType type) const -> std::vector<std::shared_ptr<Node>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Node>> result;
    auto it = m_typeIndex.find(type);
    if (it != m_typeIndex.end()) {
        for (uint64_t id : it->second) {
            result.push_back(m_nodes.at(id));
        }
    }
    return result;
}

auto Graph::getAllNodes() const -> std::vector<std::shared_ptr<Node>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Node>> result;
    result.reserve(m_nodes.size());
    for (const auto& [id, node] : m_nodes) {
        result.push_back(node);
    }
    return result;
}

auto Graph::getNodeCount() const -> size_t {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_nodes.size();
}

auto Graph::addEdge(uint64_t sourceId, uint64_t targetId, EdgeType type) -> std::shared_ptr<Edge> {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (m_nodes.find(sourceId) == m_nodes.end() || m_nodes.find(targetId) == m_nodes.end()) {
        spdlog::warn("Cannot add edge: node not found");
        return nullptr;
    }

    uint64_t id = m_nextId++;
    auto edge = std::make_shared<Edge>(id, sourceId, targetId, type);
    m_edges[id] = edge;
    m_adjacencyList[sourceId].insert(id);
    if (edge->isBidirectional()) {
        m_adjacencyList[targetId].insert(id);
    }
    spdlog::trace("Added edge {} from {} to {}", id, sourceId, targetId);
    return edge;
}

void Graph::removeEdge(uint64_t id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_edges.find(id);
    if (it == m_edges.end()) return;

    auto sourceId = it->second->getSourceId();
    auto targetId = it->second->getTargetId();

    auto adjIt = m_adjacencyList.find(sourceId);
    if (adjIt != m_adjacencyList.end()) {
        adjIt->second.erase(id);
    }
    if (it->second->isBidirectional()) {
        adjIt = m_adjacencyList.find(targetId);
        if (adjIt != m_adjacencyList.end()) {
            adjIt->second.erase(id);
        }
    }

    m_edges.erase(it);
}

void Graph::removeEdgesBetween(uint64_t sourceId, uint64_t targetId) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    auto adjIt = m_adjacencyList.find(sourceId);
    if (adjIt == m_adjacencyList.end()) return;

    std::vector<uint64_t> toRemove;
    for (uint64_t edgeId : adjIt->second) {
        auto edge = m_edges[edgeId];
        if (edge->getTargetId() == targetId) {
            toRemove.push_back(edgeId);
        }
    }
    for (uint64_t edgeId : toRemove) {
        m_edges.erase(edgeId);
        adjIt->second.erase(edgeId);
    }
}

auto Graph::getEdge(uint64_t id) const -> std::shared_ptr<Edge> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_edges.find(id);
    if (it != m_edges.end()) {
        return it->second;
    }
    return nullptr;
}

auto Graph::getEdgesFrom(uint64_t nodeId) const -> std::vector<std::shared_ptr<Edge>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Edge>> result;
    auto adjIt = m_adjacencyList.find(nodeId);
    if (adjIt != m_adjacencyList.end()) {
        for (uint64_t edgeId : adjIt->second) {
            result.push_back(m_edges.at(edgeId));
        }
    }
    return result;
}

auto Graph::getEdgesTo(uint64_t nodeId) const -> std::vector<std::shared_ptr<Edge>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Edge>> result;
    for (const auto& [id, edge] : m_edges) {
        if (edge->getTargetId() == nodeId) {
            result.push_back(edge);
        }
    }
    return result;
}

auto Graph::getAllEdges() const -> std::vector<std::shared_ptr<Edge>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Edge>> result;
    result.reserve(m_edges.size());
    for (const auto& [id, edge] : m_edges) {
        result.push_back(edge);
    }
    return result;
}

auto Graph::getEdgeCount() const -> size_t {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_edges.size();
}

void Graph::updateFromEvent(const engine::Event& event) {
    // Simplified: create/update nodes based on event type
    // Full implementation would map all event types to graph operations
    switch (event.getType()) {
        case engine::EventType::ProcessExec: {
            // Would extract PID, comm from event data and create/update node
            break;
        }
        default:
            break;
    }
}

void Graph::pruneInactiveNodes(std::chrono::seconds timeout) {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    std::vector<uint64_t> toRemove;
    for (const auto& [id, node] : m_nodes) {
        if (now - node->getLastUpdate() > timeout) {
            toRemove.push_back(id);
        }
    }
    lock.unlock();
    for (uint64_t id : toRemove) {
        removeNode(id);
    }
}

void Graph::clear() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_nodes.clear();
    m_edges.clear();
    m_adjacencyList.clear();
    m_typeIndex.clear();
}

void Graph::traverseBFS(uint64_t startId, std::function<void(std::shared_ptr<Node>)> visitor) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_nodes.find(startId) == m_nodes.end()) return;

    std::queue<uint64_t> queue;
    std::unordered_set<uint64_t> visited;
    queue.push(startId);
    visited.insert(startId);

    while (!queue.empty()) {
        uint64_t current = queue.front();
        queue.pop();
        visitor(m_nodes.at(current));

        auto adjIt = m_adjacencyList.find(current);
        if (adjIt != m_adjacencyList.end()) {
            for (uint64_t edgeId : adjIt->second) {
                auto edge = m_edges.at(edgeId);
                uint64_t neighbor = edge->getTargetId();
                if (visited.insert(neighbor).second) {
                    queue.push(neighbor);
                }
            }
        }
    }
}

void Graph::traverseDFS(uint64_t startId, std::function<void(std::shared_ptr<Node>)> visitor) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_nodes.find(startId) == m_nodes.end()) return;

    std::function<void(uint64_t, std::unordered_set<uint64_t>&)> dfs = 
        [&](uint64_t current, std::unordered_set<uint64_t>& visited) {
            visitor(m_nodes.at(current));
            auto adjIt = m_adjacencyList.find(current);
            if (adjIt != m_adjacencyList.end()) {
                for (uint64_t edgeId : adjIt->second) {
                    auto edge = m_edges.at(edgeId);
                    uint64_t neighbor = edge->getTargetId();
                    if (visited.insert(neighbor).second) {
                        dfs(neighbor, visited);
                    }
                }
            }
        };

    std::unordered_set<uint64_t> visited;
    visited.insert(startId);
    dfs(startId, visited);
}

auto Graph::findPath(uint64_t from, uint64_t to) const -> std::vector<uint64_t> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_nodes.find(from) == m_nodes.end() || m_nodes.find(to) == m_nodes.end()) {
        return {};
    }

    std::queue<uint64_t> queue;
    std::unordered_map<uint64_t, uint64_t> parent;
    queue.push(from);
    parent[from] = 0;

    while (!queue.empty()) {
        uint64_t current = queue.front();
        queue.pop();

        if (current == to) {
            std::vector<uint64_t> path;
            while (current != 0) {
                path.push_back(current);
                current = parent[current];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        auto adjIt = m_adjacencyList.find(current);
        if (adjIt != m_adjacencyList.end()) {
            for (uint64_t edgeId : adjIt->second) {
                auto edge = m_edges.at(edgeId);
                uint64_t neighbor = edge->getTargetId();
                if (parent.find(neighbor) == parent.end()) {
                    parent[neighbor] = current;
                    queue.push(neighbor);
                }
            }
        }
    }
    return {};
}

auto Graph::getNeighbors(uint64_t nodeId) const -> std::vector<std::shared_ptr<Node>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Node>> result;
    auto adjIt = m_adjacencyList.find(nodeId);
    if (adjIt != m_adjacencyList.end()) {
        for (uint64_t edgeId : adjIt->second) {
            auto edge = m_edges.at(edgeId);
            result.push_back(m_nodes.at(edge->getTargetId()));
        }
    }
    return result;
}

auto Graph::getConnectedComponents() const -> std::vector<std::vector<uint64_t>> {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::vector<uint64_t>> components;
    std::unordered_set<uint64_t> visited;

    for (const auto& [id, node] : m_nodes) {
        if (visited.find(id) != visited.end()) continue;

        std::vector<uint64_t> component;
        std::queue<uint64_t> queue;
        queue.push(id);
        visited.insert(id);

        while (!queue.empty()) {
            uint64_t current = queue.front();
            queue.pop();
            component.push_back(current);

            auto adjIt = m_adjacencyList.find(current);
            if (adjIt != m_adjacencyList.end()) {
                for (uint64_t edgeId : adjIt->second) {
                    auto edge = m_edges.at(edgeId);
                    uint64_t neighbor = edge->getTargetId();
                    if (visited.insert(neighbor).second) {
                        queue.push(neighbor);
                    }
                }
            }
        }
        components.push_back(std::move(component));
    }
    return components;
}

} // namespace xray::graph
