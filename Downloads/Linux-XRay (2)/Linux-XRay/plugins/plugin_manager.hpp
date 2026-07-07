#pragma once

#include "plugins/plugin_sdk.hpp"
#include <vector>
#include <memory>
#include <map>
#include <filesystem>

namespace xray::plugins {

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    bool loadPlugin(const std::filesystem::path& path);
    bool unloadPlugin(const std::string& name);
    void loadAllPlugins(const std::filesystem::path& directory);

    [[nodiscard]] auto getLoadedPlugins() const -> std::vector<PluginInfo>;
    [[nodiscard]] auto getPlugin(const std::string& name) const -> std::shared_ptr<PluginInterface>;

    void enablePlugin(const std::string& name);
    void disablePlugin(const std::string& name);

    void initializeAll();
    void shutdownAll();

    [[nodiscard]] auto getCollectors() const -> std::vector<std::shared_ptr<CollectorPlugin>>;
    [[nodiscard]] auto getExporters() const -> std::vector<std::shared_ptr<ExporterPlugin>>;

private:
    struct PluginHandle {
        std::shared_ptr<PluginInterface> plugin;
        void* libraryHandle{nullptr};
    };

    std::map<std::string, PluginHandle> m_plugins;
};

} // namespace xray::plugins
