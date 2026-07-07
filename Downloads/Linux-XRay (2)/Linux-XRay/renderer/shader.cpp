#include "renderer/shader.hpp"
#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <fstream>
#include <sstream>

namespace xray::renderer {

Shader::Shader() = default;

Shader::~Shader() {
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
    }
}

Shader::Shader(Shader&& other) noexcept : m_programId(other.m_programId) {
    other.m_programId = 0;
}

auto Shader::operator=(Shader&& other) noexcept -> Shader& {
    if (this != &other) {
        if (m_programId != 0) {
            glDeleteProgram(m_programId);
        }
        m_programId = other.m_programId;
        other.m_programId = 0;
    }
    return *this;
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertexShader, vertexSource)) {
        glDeleteShader(vertexShader);
        return false;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragmentShader, fragmentSource)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertexShader);
    glAttachShader(m_programId, fragmentShader);
    glLinkProgram(m_programId);

    int success;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
        spdlog::error("Shader linking failed: {}", infoLog);
        glDeleteProgram(m_programId);
        m_programId = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success;
}

bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vShaderFile(vertexPath);
    std::ifstream fShaderFile(fragmentPath);

    if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
        spdlog::error("Failed to open shader files");
        return false;
    }

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    return loadFromSource(vShaderStream.str(), fShaderStream.str());
}

void Shader::use() const {
    glUseProgram(m_programId);
}

bool Shader::compileShader(unsigned int shaderId, const std::string& source) {
    const char* src = source.c_str();
    glShaderSource(shaderId, 1, &src, nullptr);
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
        spdlog::error("Shader compilation failed: {}", infoLog);
        return false;
    }
    return true;
}

auto Shader::getUniformLocation(const std::string& name) const -> int {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    int location = glGetUniformLocation(m_programId, name.c_str());
    m_uniformCache[name] = location;
    return location;
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

} // namespace xray::renderer
