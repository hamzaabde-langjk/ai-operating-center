#pragma once

#include "graph/node.hpp"
#include "graph/edge.hpp"
#include "engine/event.hpp"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <functional>

namespace xray::graph {

class Graph {
public:
    Graph();

    auto addNode(NodeType type, std::string name) -> std::shared_ptr<Node>;
    void removeNode(uint64_t id);
    [[nodiscard]] auto getNode(uint64_t id) const -> std::shared_ptr<Node>;
    [[nodiscard]] auto getNodesByType(NodeType type) const -> std::vector<std::shared_ptr<Node>>;
    [[nodiscard]] auto getAllNodes() const -> std::vector<std::shared_ptr<Node>>;
    [[nodiscard]] auto getNodeCount() const -> size_t;

    auto addEdge(uint64_t sourceId, uint64_t targetId, EdgeType type) -> std::shared_ptr<Edge>;
    void removeEdge(uint64_t id);
    void removeEdgesBetween(uint64_t sourceId, uint64_t targetId);
    [[nodiscard]] auto getEdge(uint64_t id) const -> std::shared_ptr<Edge>;
    [[nodiscard]] auto getEdgesFrom(uint64_t nodeId) const -> std::vector<std::shared_ptr<Edge>>;
    [[nodiscard]] auto getEdgesTo(uint64_t nodeId) const -> std::vector<std::shared_ptr<Edge>>;
    [[nodiscard]] auto getAllEdges() const -> std::vector<std::shared_ptr<Edge>>;
    [[nodiscard]] auto getEdgeCount() const -> size_t;

    void updateFromEvent(const engine::Event& event);
    void pruneInactiveNodes(std::chrono::seconds timeout);
    void clear();

    void traverseBFS(uint64_t startId, std::function<void(std::shared_ptr<Node>)> visitor) const;
    void traverseDFS(uint64_t startId, std::function<void(std::shared_ptr<Node>)> visitor) const;

    [[nodiscard]] auto findPath(uint64_t from, uint64_t to) const -> std::vector<uint64_t>;
    [[nodiscard]] auto getNeighbors(uint64_t nodeId) const -> std::vector<std::shared_ptr<Node>>;
    [[nodiscard]] auto getConnectedComponents() const -> std::vector<std::vector<uint64_t>>;

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<uint64_t, std::shared_ptr<Node>> m_nodes;
    std::unordered_map<uint64_t, std::shared_ptr<Edge>> m_edges;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> m_adjacencyList;
    std::unordered_map<NodeType, std::unordered_set<uint64_t>> m_typeIndex;
    std::atomic<uint64_t> m_nextId{1};
};

} // namespace xray::graph
