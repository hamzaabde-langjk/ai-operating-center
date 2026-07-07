#include "xray/sdk/plugin_interface.hpp"
#include <spdlog/spdlog.h>
#include <fstream>

using namespace xray::plugins;
using namespace xray::engine;
using namespace xray::graph;

class SampleCollectorPlugin : public CollectorPlugin {
public:
    [[nodiscard]] auto getInfo() const -> PluginInfo override {
        return {"SampleCollector", "1.0.0", "X-Ray Team", 
                "A sample event collector plugin", PluginType::Collector, {}};
    }

    bool initialize() override {
        spdlog::info("SampleCollector initialized");
        m_enabled = true;
        return true;
    }

    void shutdown() override {
        spdlog::info("SampleCollector shutdown");
        m_enabled = false;
    }

    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }

    void startCollection() override {
        spdlog::info("SampleCollector started collection");
    }

    void stopCollection() override {
        spdlog::info("SampleCollector stopped collection");
    }

    [[nodiscard]] auto getEvents() -> std::vector<Event> override {
        std::vector<Event> events;
        return events;
    }

private:
    bool m_enabled{false};
};

XRAY_PLUGIN_CREATE(SampleCollectorPlugin)
XRAY_PLUGIN_DESTROY()
