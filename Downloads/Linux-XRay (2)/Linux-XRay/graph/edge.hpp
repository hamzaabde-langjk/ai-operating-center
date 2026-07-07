#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <chrono>

namespace xray::graph {

class Node;

enum class EdgeType : uint32_t {
    ParentChild = 0,
    FileOperation,
    NetworkConnection,
    Dependency,
    Signal,
    MemoryMapping,
    LibraryLink,
    Communication,
    Custom
};

class Edge {
public:
    Edge(uint64_t id, uint64_t sourceId, uint64_t targetId, EdgeType type);

    [[nodiscard]] auto getId() const -> uint64_t { return m_id; }
    [[nodiscard]] auto getSourceId() const -> uint64_t { return m_sourceId; }
    [[nodiscard]] auto getTargetId() const -> uint64_t { return m_targetId; }
    [[nodiscard]] auto getType() const -> EdgeType { return m_type; }

    void setWeight(float weight) { m_weight = weight; }
    [[nodiscard]] auto getWeight() const -> float { return m_weight; }

    void setLabel(std::string label) { m_label = std::move(label); }
    [[nodiscard]] auto getLabel() const -> const std::string& { return m_label; }

    void setBidirectional(bool bidirectional) { m_bidirectional = bidirectional; }
    [[nodiscard]] auto isBidirectional() const -> bool { return m_bidirectional; }

    void updateTimestamp();
    [[nodiscard]] auto getLastUpdate() const -> std::chrono::steady_clock::time_point;

    void setActivity(float activity) { m_activity = std::clamp(activity, 0.0f, 1.0f); }
    [[nodiscard]] auto getActivity() const -> float { return m_activity; }

private:
    uint64_t m_id;
    uint64_t m_sourceId;
    uint64_t m_targetId;
    EdgeType m_type;
    float m_weight{1.0f};
    std::string m_label;
    bool m_bidirectional{false};
    std::chrono::steady_clock::time_point m_lastUpdate;
    float m_activity{0.0f};
};

} // namespace xray::graph
