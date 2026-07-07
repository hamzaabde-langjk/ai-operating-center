#pragma once

#include "engine/event.hpp"
#include "graph/graph.hpp"
#include <string>
#include <vector>
#include <memory>

namespace xray::plugins {

enum class PluginType {
    Collector,
    GraphNode,
    Panel,
    AiModel,
    Visualizer,
    Exporter
};

struct PluginInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    PluginType type;
    std::vector<std::string> dependencies;
};

class PluginInterface {
public:
    virtual ~PluginInterface() = default;

    [[nodiscard]] virtual auto getInfo() const -> PluginInfo = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

class CollectorPlugin : public PluginInterface {
public:
    virtual void startCollection() = 0;
    virtual void stopCollection() = 0;
    [[nodiscard]] virtual auto getEvents() -> std::vector<engine::Event> = 0;
};

class GraphPlugin : public PluginInterface {
public:
    virtual void onGraphUpdate(graph::Graph& graph) = 0;
    virtual void onNodeAdded(std::shared_ptr<graph::Node> node) = 0;
    virtual void onEdgeAdded(std::shared_ptr<graph::Edge> edge) = 0;
};

class ExporterPlugin : public PluginInterface {
public:
    virtual bool exportData(const std::string& path, const std::vector<engine::Event>& events) = 0;
    [[nodiscard]] virtual auto getSupportedFormats() const -> std::vector<std::string> = 0;
};

using CreatePluginFunc = PluginInterface* (*)();
using DestroyPluginFunc = void (*)(PluginInterface*);

} // namespace xray::plugins

#define XRAY_PLUGIN_EXPORT extern "C"
#define XRAY_PLUGIN_CREATE(name) XRAY_PLUGIN_EXPORT xray::plugins::PluginInterface* createPlugin() { return new name(); }
#define XRAY_PLUGIN_DESTROY() XRAY_PLUGIN_EXPORT void destroyPlugin(xray::plugins::PluginInterface* plugin) { delete plugin; }
