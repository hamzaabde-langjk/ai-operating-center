#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace xray::renderer {

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 50.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    [[nodiscard]] auto getViewMatrix() const -> glm::mat4;
    [[nodiscard]] auto getProjectionMatrix(float aspectRatio) const -> glm::mat4;
    [[nodiscard]] auto getPosition() const -> glm::vec3 { return m_position; }
    [[nodiscard]] auto getFront() const -> glm::vec3 { return m_front; }

    void processKeyboard(int direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void processMouseScroll(float yOffset);

    void setPosition(const glm::vec3& position);
    void lookAt(const glm::vec3& target);

    void setMovementSpeed(float speed) { m_movementSpeed = speed; }
    void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
    void setZoom(float zoom);

private:
    void updateCameraVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;
    float m_movementSpeed{10.0f};
    float m_mouseSensitivity{0.1f};
    float m_zoom{45.0f};
};

} // namespace xray::renderer
