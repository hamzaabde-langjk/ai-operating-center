#include "graph/node.hpp"
#include <sstream>

namespace xray::graph {

Node::Node(uint64_t id, NodeType type, std::string name)
    : m_id(id), m_type(type), m_name(std::move(name)), m_lastUpdate(std::chrono::steady_clock::now()) {}

auto Node::getDisplayName() const -> std::string {
    std::ostringstream oss;
    oss << "[" << static_cast<uint32_t>(m_type) << "] " << m_name;
    return oss.str();
}

void Node::setPosition(float x, float y, float z) {
    m_x = x;
    m_y = y;
    m_z = z;
}

auto Node::getPosition() const -> std::tuple<float, float, float> {
    return {m_x, m_y, m_z};
}

void Node::setColor(float r, float g, float b, float a) {
    m_r = std::clamp(r, 0.0f, 1.0f);
    m_g = std::clamp(g, 0.0f, 1.0f);
    m_b = std::clamp(b, 0.0f, 1.0f);
    m_a = std::clamp(a, 0.0f, 1.0f);
}

auto Node::getColor() const -> std::tuple<float, float, float, float> {
    return {m_r, m_g, m_b, m_a};
}

void Node::setMetadata(std::string key, std::string value) {
    m_metadata[std::move(key)] = std::move(value);
}

auto Node::getMetadata(const std::string& key) const -> std::optional<std::string> {
    auto it = m_metadata.find(key);
    if (it != m_metadata.end()) {
        return it->second;
    }
    return std::nullopt;
}

auto Node::getAllMetadata() const -> const std::unordered_map<std::string, std::string>& {
    return m_metadata;
}

void Node::updateTimestamp() {
    m_lastUpdate = std::chrono::steady_clock::now();
}

auto Node::getLastUpdate() const -> std::chrono::steady_clock::time_point {
    return m_lastUpdate;
}

} // namespace xray::graph
