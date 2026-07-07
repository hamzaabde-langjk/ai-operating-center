#include "plugins/plugin_manager.hpp"
#include <spdlog/spdlog.h>
#include <dlfcn.h>

namespace xray::plugins {

PluginManager::PluginManager() = default;

PluginManager::~PluginManager() {
    shutdownAll();
}

bool PluginManager::loadPlugin(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Plugin not found: {}", path.string());
        return false;
    }

    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        spdlog::error("Failed to load plugin {}: {}", path.string(), dlerror());
        return false;
    }

    auto createFunc = reinterpret_cast<CreatePluginFunc>(dlsym(handle, "createPlugin"));
    if (!createFunc) {
        spdlog::error("Plugin {} missing createPlugin function", path.string());
        dlclose(handle);
        return false;
    }

    auto plugin = std::shared_ptr<PluginInterface>(createFunc());
    auto info = plugin->getInfo();

    spdlog::info("Loaded plugin: {} v{} by {}", info.name, info.version, info.author);

    m_plugins[info.name] = {plugin, handle};
    return true;
}

bool PluginManager::unloadPlugin(const std::string& name) {
    auto it = m_plugins.find(name);
    if (it == m_plugins.end()) {
        return false;
    }

    it->second.plugin->shutdown();
    if (it->second.libraryHandle) {
        dlclose(it->second.libraryHandle);
    }
    m_plugins.erase(it);

    spdlog::info("Unloaded plugin: {}", name);
    return true;
}

void PluginManager::loadAllPlugins(const std::filesystem::path& directory) {
    if (!std::filesystem::exists(directory)) {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".so") {
            loadPlugin(entry.path());
        }
    }
}

auto PluginManager::getLoadedPlugins() const -> std::vector<PluginInfo> {
    std::vector<PluginInfo> result;
    for (const auto& [name, handle] : m_plugins) {
        result.push_back(handle.plugin->getInfo());
    }
    return result;
}

auto PluginManager::getPlugin(const std::string& name) const -> std::shared_ptr<PluginInterface> {
    auto it = m_plugins.find(name);
    if (it != m_plugins.end()) {
        return it->second.plugin;
    }
    return nullptr;
}

void PluginManager::enablePlugin(const std::string& name) {
    auto plugin = getPlugin(name);
    if (plugin) {
        plugin->setEnabled(true);
    }
}

void PluginManager::disablePlugin(const std::string& name) {
    auto plugin = getPlugin(name);
    if (plugin) {
        plugin->setEnabled(false);
    }
}

void PluginManager::initializeAll() {
    for (const auto& [name, handle] : m_plugins) {
        if (!handle.plugin->initialize()) {
            spdlog::error("Failed to initialize plugin: {}", name);
        }
    }
}

void PluginManager::shutdownAll() {
    for (const auto& [name, handle] : m_plugins) {
        handle.plugin->shutdown();
        if (handle.libraryHandle) {
            dlclose(handle.libraryHandle);
        }
    }
    m_plugins.clear();
}

auto PluginManager::getCollectors() const -> std::vector<std::shared_ptr<CollectorPlugin>> {
    std::vector<std::shared_ptr<CollectorPlugin>> result;
    for (const auto& [name, handle] : m_plugins) {
        auto collector = std::dynamic_pointer_cast<CollectorPlugin>(handle.plugin);
        if (collector) {
            result.push_back(collector);
        }
    }
    return result;
}

auto PluginManager::getExporters() const -> std::vector<std::shared_ptr<ExporterPlugin>> {
    std::vector<std::shared_ptr<ExporterPlugin>> result;
    for (const auto& [name, handle] : m_plugins) {
        auto exporter = std::dynamic_pointer_cast<ExporterPlugin>(handle.plugin);
        if (exporter) {
            result.push_back(exporter);
        }
    }
    return result;
}

} // namespace xray::plugins
