#include "renderer/camera.hpp"
#include <algorithm>

namespace xray::renderer {

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position), m_worldUp(up), m_yaw(yaw), m_pitch(pitch) {
    updateCameraVectors();
}

auto Camera::getViewMatrix() const -> glm::mat4 {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

auto Camera::getProjectionMatrix(float aspectRatio) const -> glm::mat4 {
    return glm::perspective(glm::radians(m_zoom), aspectRatio, 0.1f, 1000.0f);
}

void Camera::processKeyboard(int direction, float deltaTime) {
    float velocity = m_movementSpeed * deltaTime;
    if (direction == 0) // Forward
        m_position += m_front * velocity;
    if (direction == 1) // Backward
        m_position -= m_front * velocity;
    if (direction == 2) // Left
        m_position -= m_right * velocity;
    if (direction == 3) // Right
        m_position += m_right * velocity;
    if (direction == 4) // Up
        m_position += m_worldUp * velocity;
    if (direction == 5) // Down
        m_position -= m_worldUp * velocity;
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;

    if (constrainPitch) {
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    m_zoom -= yOffset;
    m_zoom = std::clamp(m_zoom, 1.0f, 90.0f);
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
}

void Camera::lookAt(const glm::vec3& target) {
    m_front = glm::normalize(target - m_position);
    m_pitch = glm::degrees(asin(m_front.y));
    m_yaw = glm::degrees(atan2(m_front.z, m_front.x));
    updateCameraVectors();
}

void Camera::setZoom(float zoom) {
    m_zoom = std::clamp(zoom, 1.0f, 90.0f);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace xray::renderer
