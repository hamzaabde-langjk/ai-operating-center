#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace xray::renderer {

class Shader {
public:
    Shader();
    ~Shader();

    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;
    Shader(Shader&& other) noexcept;
    auto operator=(Shader&& other) noexcept -> Shader&;

    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);

    void use() const;
    [[nodiscard]] auto isValid() const -> bool { return m_programId != 0; }

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int m_programId{0};
    mutable std::unordered_map<std::string, int> m_uniformCache;

    bool compileShader(unsigned int shaderId, const std::string& source);
    auto getUniformLocation(const std::string& name) const -> int;
};

} // namespace xray::renderer
