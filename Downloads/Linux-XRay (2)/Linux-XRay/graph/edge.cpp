#include "graph/edge.hpp"

namespace xray::graph {

Edge::Edge(uint64_t id, uint64_t sourceId, uint64_t targetId, EdgeType type)
    : m_id(id), m_sourceId(sourceId), m_targetId(targetId), m_type(type),
      m_lastUpdate(std::chrono::steady_clock::now()) {}

void Edge::updateTimestamp() {
    m_lastUpdate = std::chrono::steady_clock::now();
}

auto Edge::getLastUpdate() const -> std::chrono::steady_clock::time_point {
    return m_lastUpdate;
}

} // namespace xray::graph
