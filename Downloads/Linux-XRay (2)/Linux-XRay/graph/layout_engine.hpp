#pragma once

#include "graph/graph.hpp"
#include <random>
#include <cmath>

namespace xray::graph {

class LayoutEngine {
public:
    virtual ~LayoutEngine() = default;
    virtual void apply(Graph& graph) = 0;
    virtual void step(Graph& graph) = 0;
    [[nodiscard]] virtual auto isConverged() const -> bool = 0;
};

class ForceDirectedLayout : public LayoutEngine {
public:
    ForceDirectedLayout(float repulsion = 100.0f, float attraction = 0.01f, 
                       float damping = 0.9f);

    void apply(Graph& graph) override;
    void step(Graph& graph) override;
    [[nodiscard]] auto isConverged() const -> bool override;

    void setRepulsion(float repulsion) { m_repulsion = repulsion; }
    void setAttraction(float attraction) { m_attraction = attraction; }
    void setMaxIterations(size_t max) { m_maxIterations = max; }

private:
    float m_repulsion;
    float m_attraction;
    float m_damping;
    size_t m_maxIterations{1000};
    size_t m_currentIteration{0};
    float m_threshold{0.01f};
    std::mt19937 m_rng;
};

class SpiralGalaxyLayout : public LayoutEngine {
public:
    void apply(Graph& graph) override;
    void step(Graph& graph) override;
    [[nodiscard]] auto isConverged() const -> bool override { return true; }

    void setCenter(float x, float y, float z);
    void setSpiralTightness(float tightness) { m_tightness = tightness; }

private:
    float m_cx{0.0f}, m_cy{0.0f}, m_cz{0.0f};
    float m_tightness{0.5f};
};

class HierarchicalLayout : public LayoutEngine {
public:
    void apply(Graph& graph) override;
    void step(Graph& graph) override;
    [[nodiscard]] auto isConverged() const -> bool override { return true; }

    void setLevelSpacing(float spacing) { m_levelSpacing = spacing; }
    void setNodeSpacing(float spacing) { m_nodeSpacing = spacing; }

private:
    float m_levelSpacing{5.0f};
    float m_nodeSpacing{3.0f};
};

} // namespace xray::graph
