#include "graph/layout_engine.hpp"
#include <spdlog/spdlog.h>
#include <cmath>

namespace xray::graph {

// Force Directed Layout
ForceDirectedLayout::ForceDirectedLayout(float repulsion, float attraction, float damping)
    : m_repulsion(repulsion), m_attraction(attraction), m_damping(damping),
      m_rng(std::random_device{}()) {}

void ForceDirectedLayout::apply(Graph& graph) {
    m_currentIteration = 0;

    // Random initial positions
    std::uniform_real_distribution<float> dist(-50.0f, 50.0f);
    auto nodes = graph.getAllNodes();
    for (auto& node : nodes) {
        auto [x, y, z] = node->getPosition();
        if (x == 0.0f && y == 0.0f && z == 0.0f) {
            node->setPosition(dist(m_rng), dist(m_rng), dist(m_rng));
        }
    }
}

void ForceDirectedLayout::step(Graph& graph) {
    if (m_currentIteration >= m_maxIterations) return;

    auto nodes = graph.getAllNodes();
    std::unordered_map<uint64_t, std::tuple<float, float, float>> displacements;

    // Repulsion (Coulomb's law)
    for (size_t i = 0; i < nodes.size(); ++i) {
        float dx = 0.0f, dy = 0.0f, dz = 0.0f;
        auto [x1, y1, z1] = nodes[i]->getPosition();

        for (size_t j = 0; j < nodes.size(); ++j) {
            if (i == j) continue;
            auto [x2, y2, z2] = nodes[j]->getPosition();

            float diffX = x1 - x2;
            float diffY = y1 - y2;
            float diffZ = z1 - z2;
            float dist = std::sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);

            if (dist < 0.1f) dist = 0.1f;

            float force = m_repulsion / (dist * dist);
            dx += (diffX / dist) * force;
            dy += (diffY / dist) * force;
            dz += (diffZ / dist) * force;
        }

        displacements[nodes[i]->getId()] = {dx, dy, dz};
    }

    // Attraction (Hooke's law)
    auto edges = graph.getAllEdges();
    for (const auto& edge : edges) {
        auto source = graph.getNode(edge->getSourceId());
        auto target = graph.getNode(edge->getTargetId());
        if (!source || !target) continue;

        auto [sx, sy, sz] = source->getPosition();
        auto [tx, ty, tz] = target->getPosition();

        float diffX = tx - sx;
        float diffY = ty - sy;
        float diffZ = tz - sz;
        float dist = std::sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);

        if (dist < 0.1f) dist = 0.1f;

        float force = m_attraction * dist;
        float fx = (diffX / dist) * force;
        float fy = (diffY / dist) * force;
        float fz = (diffZ / dist) * force;

        auto& [sdx, sdy, sdz] = displacements[source->getId()];
        sdx += fx;
        sdy += fy;
        sdz += fz;

        auto& [tdx, tdy, tdz] = displacements[target->getId()];
        tdx -= fx;
        tdy -= fy;
        tdz -= fz;
    }

    // Apply displacements with damping
    float maxDisplacement = 0.0f;
    for (auto& node : nodes) {
        auto [dx, dy, dz] = displacements[node->getId()];
        auto [x, y, z] = node->getPosition();

        dx *= m_damping;
        dy *= m_damping;
        dz *= m_damping;

        node->setPosition(x + dx, y + dy, z + dz);

        float disp = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (disp > maxDisplacement) maxDisplacement = disp;
    }

    m_currentIteration++;

    if (maxDisplacement < m_threshold) {
        m_currentIteration = m_maxIterations; // Converged
    }
}

auto ForceDirectedLayout::isConverged() const -> bool {
    return m_currentIteration >= m_maxIterations;
}

// Spiral Galaxy Layout
void SpiralGalaxyLayout::apply(Graph& graph) {
    auto nodes = graph.getAllNodes();
    float angleStep = 2.0f * 3.14159f / std::max(size_t(1), nodes.size());
    float radius = 0.0f;

    for (size_t i = 0; i < nodes.size(); ++i) {
        float angle = i * angleStep * m_tightness;
        radius += 2.0f;
        float x = m_cx + radius * std::cos(angle);
        float y = m_cy + radius * std::sin(angle);
        float z = m_cz + (i % 2 == 0 ? 1.0f : -1.0f) * (radius * 0.1f);
        nodes[i]->setPosition(x, y, z);
    }
}

void SpiralGalaxyLayout::step(Graph& graph) {
    // Static layout, no stepping needed
}

void SpiralGalaxyLayout::setCenter(float x, float y, float z) {
    m_cx = x;
    m_cy = y;
    m_cz = z;
}

// Hierarchical Layout
void HierarchicalLayout::apply(Graph& graph) {
    // Group nodes by type and arrange hierarchically
    auto nodes = graph.getAllNodes();

    std::map<NodeType, std::vector<std::shared_ptr<Node>>> typeGroups;
    for (const auto& node : nodes) {
        typeGroups[node->getType()].push_back(node);
    }

    float yOffset = 0.0f;
    for (auto& [type, group] : typeGroups) {
        float xOffset = -(group.size() * m_nodeSpacing) / 2.0f;
        for (size_t i = 0; i < group.size(); ++i) {
            group[i]->setPosition(xOffset + i * m_nodeSpacing, yOffset, 0.0f);
        }
        yOffset += m_levelSpacing;
    }
}

void HierarchicalLayout::step(Graph& graph) {
    // Static layout
}

} // namespace xray::graph
