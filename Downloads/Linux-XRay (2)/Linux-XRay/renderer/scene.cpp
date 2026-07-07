#include "renderer/scene.hpp"
#include <spdlog/spdlog.h>

namespace xray::renderer {

Scene::Scene() {
    // Default colors for node types
    m_nodeColors[graph::NodeType::Process] = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);    // Blue
    m_nodeColors[graph::NodeType::File] = glm::vec4(0.4f, 0.8f, 0.4f, 1.0f);       // Green
    m_nodeColors[graph::NodeType::Socket] = glm::vec4(1.0f, 0.6f, 0.2f, 1.0f);     // Orange
    m_nodeColors[graph::NodeType::Container] = glm::vec4(0.8f, 0.3f, 0.8f, 1.0f); // Purple
    m_nodeColors[graph::NodeType::Kernel] = glm::vec4(1.0f, 0.9f, 0.2f, 1.0f);     // Yellow (Sun)
    m_nodeColors[graph::NodeType::Directory] = glm::vec4(0.2f, 0.3f, 0.6f, 1.0f); // Dark Blue
    m_nodeColors[graph::NodeType::IpAddress] = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);  // Red
    m_nodeColors[graph::NodeType::User] = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);       // Gray
    m_nodeColors[graph::NodeType::Service] = glm::vec4(0.2f, 0.8f, 0.8f, 1.0f);    // Cyan
    m_nodeColors[graph::NodeType::Device] = glm::vec4(0.8f, 0.8f, 0.2f, 1.0f);     // Yellow-Green

    // Default edge colors
    m_edgeColors[graph::EdgeType::ParentChild] = glm::vec4(0.5f, 0.5f, 0.5f, 0.6f);
    m_edgeColors[graph::EdgeType::FileOperation] = glm::vec4(0.4f, 0.8f, 0.4f, 0.5f);
    m_edgeColors[graph::EdgeType::NetworkConnection] = glm::vec4(1.0f, 0.6f, 0.2f, 0.7f);
    m_edgeColors[graph::EdgeType::Dependency] = glm::vec4(0.6f, 0.6f, 0.6f, 0.4f);
}

Scene::~Scene() = default;

void Scene::updateFromGraph(const graph::Graph& graph) {
    m_nodes.clear();
    m_edges.clear();

    auto nodes = graph.getAllNodes();
    m_nodes.reserve(nodes.size());

    for (const auto& node : nodes) {
        RenderNode rn;
        rn.graphNodeId = node->getId();
        auto [x, y, z] = node->getPosition();
        rn.position = glm::vec3(x, y, z);
        auto [r, g, b, a] = node->getColor();
        rn.color = glm::vec4(r, g, b, a);
        rn.size = node->getSize();
        rn.activity = node->getActivityLevel();
        rn.label = node->getName();

        // Apply type-based color if not customized
        auto colorIt = m_nodeColors.find(node->getType());
        if (colorIt != m_nodeColors.end() && rn.color == glm::vec4(1.0f)) {
            rn.color = colorIt->second;
        }

        // Apply highlight
        if (m_highlightedNodes.count(node->getId())) {
            rn.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow highlight
            rn.size *= 1.5f;
        }

        m_nodes.push_back(std::move(rn));
    }

    auto edges = graph.getAllEdges();
    m_edges.reserve(edges.size());

    for (const auto& edge : edges) {
        auto source = graph.getNode(edge->getSourceId());
        auto target = graph.getNode(edge->getTargetId());
        if (!source || !target) continue;

        RenderEdge re;
        re.graphEdgeId = edge->getId();
        auto [sx, sy, sz] = source->getPosition();
        auto [tx, ty, tz] = target->getPosition();
        re.start = glm::vec3(sx, sy, sz);
        re.end = glm::vec3(tx, ty, tz);
        re.thickness = edge->getWeight();
        re.activity = edge->getActivity();

        auto colorIt = m_edgeColors.find(edge->getType());
        if (colorIt != m_edgeColors.end()) {
            re.color = colorIt->second;
        } else {
            re.color = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
        }

        m_edges.push_back(std::move(re));
    }
}

void Scene::updateParticles(float deltaTime) {
    for (auto& particle : m_particles) {
        particle.position += particle.velocity * deltaTime;
        particle.life -= deltaTime;
        particle.color.a = std::max(0.0f, particle.life);
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                      [](const auto& p) { return p.life <= 0.0f; }),
        m_particles.end()
    );
}

void Scene::addParticle(const Particle& particle) {
    m_particles.push_back(particle);
    if (m_particles.size() > 10000) {
        m_particles.erase(m_particles.begin());
    }
}

void Scene::clearParticles() {
    m_particles.clear();
}

void Scene::setNodeColorByType(graph::NodeType type, const glm::vec4& color) {
    m_nodeColors[type] = color;
}

void Scene::setEdgeColorByType(graph::EdgeType type, const glm::vec4& color) {
    m_edgeColors[type] = color;
}

void Scene::highlightNode(uint64_t nodeId, const glm::vec4& color) {
    m_highlightedNodes.insert(nodeId);
}

void Scene::clearHighlights() {
    m_highlightedNodes.clear();
}

} // namespace xray::renderer
