#pragma once

#include <glm/glm.hpp>
#include <array>

namespace mc {

class Frustum {
public:
    void update(const glm::mat4& vp) {
        auto row = [&](int i) -> glm::vec4 {
            return { vp[0][i], vp[1][i], vp[2][i], vp[3][i] };
        };
        m_planes[0] = row(3) + row(0); // left
        m_planes[1] = row(3) - row(0); // right
        m_planes[2] = row(3) + row(1); // bottom
        m_planes[3] = row(3) - row(1); // top
        m_planes[4] = row(3) + row(2); // near
        m_planes[5] = row(3) - row(2); // far

        for (auto& p : m_planes) {
            float len = glm::length(glm::vec3(p));
            if (len > 0.0f) p /= len;
        }
    }


    [[nodiscard]] bool testAABB(const glm::vec3& minPt,
                                 const glm::vec3& maxPt) const
    {
        for (const auto& plane : m_planes) {
            glm::vec3 positive = minPt;
            if (plane.x >= 0.0f) positive.x = maxPt.x;
            if (plane.y >= 0.0f) positive.y = maxPt.y;
            if (plane.z >= 0.0f) positive.z = maxPt.z;

            if (glm::dot(glm::vec3(plane), positive) + plane.w < 0.0f)
                return false; 
        }
        return true;
    }

private:
    std::array<glm::vec4, 6> m_planes{};
};

} // namespace mc
