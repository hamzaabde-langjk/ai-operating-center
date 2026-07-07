# Plugin Development Example

## Creating a Custom Collector

```cpp
#include "xray/sdk/plugin_interface.hpp"
#include <spdlog/spdlog.h>

using namespace xray::plugins;
using namespace xray::engine;

class MyCollector : public CollectorPlugin {
public:
    PluginInfo getInfo() const override {
        return {"MyCollector", "1.0.0", "Your Name",
                "Custom event collector", PluginType::Collector, {}};
    }

    bool initialize() override {
        m_enabled = true;
        return true;
    }

    void shutdown() override { m_enabled = false; }
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override { m_enabled = enabled; }

    void startCollection() override {
        spdlog::info("MyCollector started");
    }

    void stopCollection() override {
        spdlog::info("MyCollector stopped");
    }

    std::vector<Event> getEvents() override {
        std::vector<Event> events;
        // Collect your custom events here
        return events;
    }

private:
    bool m_enabled = false;
};

XRAY_PLUGIN_CREATE(MyCollector)
XRAY_PLUGIN_DESTROY()
```

## Building the Plugin

```bash
g++ -shared -fPIC -I/path/to/sdk/include my_plugin.cpp -o my_plugin.so
```

## Loading the Plugin

```bash
xray --plugin-dir=/path/to/plugins
```
