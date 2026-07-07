#pragma once

#include "graph/graph.hpp"
#include <functional>

namespace xray::graph {

class GraphRenderer {
public:
    virtual ~GraphRenderer() = default;

    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual void render(const Graph& graph) = 0;
    virtual void updateNodePosition(uint64_t nodeId, float x, float y, float z) = 0;
    virtual void setCamera(float x, float y, float z, float pitch, float yaw) = 0;

    virtual void onNodeClick(std::function<void(uint64_t)> callback) = 0;
    virtual void onNodeHover(std::function<void(uint64_t)> callback) = 0;
};

} // namespace xray::graph
