#include "core/Camera.h"
#include "core/Window.h"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace mc {

void Camera::processInput(const Window& window, float dt) {
    float speed = moveSpeed * dt;
    if (window.isKeyDown(GLFW_KEY_LEFT_CONTROL))
        speed *= sprintMultiplier;

    if (window.isKeyDown(GLFW_KEY_W)) position += m_front * speed;
    if (window.isKeyDown(GLFW_KEY_S)) position -= m_front * speed;
    if (window.isKeyDown(GLFW_KEY_A)) position -= m_right * speed;
    if (window.isKeyDown(GLFW_KEY_D)) position += m_right * speed;
    if (window.isKeyDown(GLFW_KEY_SPACE))      position.y += speed;
    if (window.isKeyDown(GLFW_KEY_LEFT_SHIFT))  position.y -= speed;
}

void Camera::processMouseMovement(double dx, double dy) {
    yaw   += static_cast<float>(dx) * sensitivity;
    pitch += static_cast<float>(dy) * sensitivity;
    pitch  = std::clamp(pitch, -89.0f, 89.0f);
    updateVectors();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position, position + m_front, m_up);
}

glm::mat4 Camera::projectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

void Camera::updateVectors() {
    float yawRad   = glm::radians(yaw);
    float pitchRad = glm::radians(pitch);

    m_front = glm::normalize(glm::vec3{
        std::cos(yawRad) * std::cos(pitchRad),
        std::sin(pitchRad),
        std::sin(yawRad) * std::cos(pitchRad)
    });
    m_right = glm::normalize(glm::cross(m_front, glm::vec3{0, 1, 0}));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace mc
