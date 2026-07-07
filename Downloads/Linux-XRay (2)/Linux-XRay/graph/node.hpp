#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>

namespace xray::graph {

enum class NodeType : uint32_t {
    Process = 0,
    Thread,
    File,
    Directory,
    Library,
    Socket,
    Domain,
    IpAddress,
    Container,
    User,
    Service,
    Device,
    Kernel,
    Custom
};

class Node {
public:
    Node(uint64_t id, NodeType type, std::string name);

    [[nodiscard]] auto getId() const -> uint64_t { return m_id; }
    [[nodiscard]] auto getType() const -> NodeType { return m_type; }
    [[nodiscard]] auto getName() const -> const std::string& { return m_name; }
    [[nodiscard]] auto getDisplayName() const -> std::string;

    void setPosition(float x, float y, float z);
    [[nodiscard]] auto getPosition() const -> std::tuple<float, float, float>;

    void setSize(float size) { m_size = size; }
    [[nodiscard]] auto getSize() const -> float { return m_size; }

    void setColor(float r, float g, float b, float a = 1.0f);
    [[nodiscard]] auto getColor() const -> std::tuple<float, float, float, float>;

    void setMetadata(std::string key, std::string value);
    [[nodiscard]] auto getMetadata(const std::string& key) const -> std::optional<std::string>;
    [[nodiscard]] auto getAllMetadata() const -> const std::unordered_map<std::string, std::string>&;

    void updateTimestamp();
    [[nodiscard]] auto getLastUpdate() const -> std::chrono::steady_clock::time_point;

    void setActivityLevel(float level) { m_activityLevel = std::clamp(level, 0.0f, 1.0f); }
    [[nodiscard]] auto getActivityLevel() const -> float { return m_activityLevel; }

private:
    uint64_t m_id;
    NodeType m_type;
    std::string m_name;
    float m_x{0.0f}, m_y{0.0f}, m_z{0.0f};
    float m_size{1.0f};
    float m_r{1.0f}, m_g{1.0f}, m_b{1.0f}, m_a{1.0f};
    std::unordered_map<std::string, std::string> m_metadata;
    std::chrono::steady_clock::time_point m_lastUpdate;
    float m_activityLevel{0.0f};
};

} // namespace xray::graph
