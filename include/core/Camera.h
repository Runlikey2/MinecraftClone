#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mc {

class Window; // forward declaration

class Camera {
public:
    glm::vec3 position{0.0f, 100.0f, 0.0f};
    float     yaw   = -90.0f;   // look toward -Z
    float     pitch =  0.0f;

    float moveSpeed       = 32.0f;   // blocks/sec
    float sprintMultiplier = 3.0f;
    float sensitivity     = 0.1f;
    float fov             = 70.0f;
    float nearPlane       = 0.1f;
    float farPlane        = 1000.0f;

    Camera() { updateVectors(); }

    void processInput(const Window& window, float dt);
    void processMouseMovement(double dx, double dy);

    [[nodiscard]] glm::mat4 viewMatrix()  const;
    [[nodiscard]] glm::mat4 projectionMatrix(float aspect) const;

    [[nodiscard]] glm::vec3 front() const { return m_front; }

private:
    glm::vec3 m_front{0, 0, -1};
    glm::vec3 m_right{1, 0,  0};
    glm::vec3 m_up   {0, 1,  0};

    void updateVectors();
};

} // namespace mc
