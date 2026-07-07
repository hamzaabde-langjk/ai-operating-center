#pragma once

#include "renderer/shader.hpp"
#include "renderer/camera.hpp"
#include "graph/graph.hpp"
#include <vector>
#include <memory>

namespace xray::renderer {

struct RenderNode {
    uint64_t graphNodeId;
    glm::vec3 position;
    glm::vec4 color;
    float size;
    float activity;
    std::string label;
};

struct RenderEdge {
    uint64_t graphEdgeId;
    glm::vec3 start;
    glm::vec3 end;
    glm::vec4 color;
    float thickness;
    float activity;
};

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;
    float size;
};

class Scene {
public:
    Scene();
    ~Scene();

    void updateFromGraph(const graph::Graph& graph);
    void updateParticles(float deltaTime);

    void addParticle(const Particle& particle);
    void clearParticles();

    [[nodiscard]] auto getNodes() const -> const std::vector<RenderNode>& { return m_nodes; }
    [[nodiscard]] auto getEdges() const -> const std::vector<RenderEdge>& { return m_edges; }
    [[nodiscard]] auto getParticles() const -> const std::vector<Particle>& { return m_particles; }

    void setNodeColorByType(graph::NodeType type, const glm::vec4& color);
    void setEdgeColorByType(graph::EdgeType type, const glm::vec4& color);

    void highlightNode(uint64_t nodeId, const glm::vec4& color);
    void clearHighlights();

private:
    std::vector<RenderNode> m_nodes;
    std::vector<RenderEdge> m_edges;
    std::vector<Particle> m_particles;
    std::unordered_map<graph::NodeType, glm::vec4> m_nodeColors;
    std::unordered_map<graph::EdgeType, glm::vec4> m_edgeColors;
    std::unordered_set<uint64_t> m_highlightedNodes;
};

} // namespace xray::renderer
