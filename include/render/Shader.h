#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace mc {

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& o) noexcept;
    Shader& operator=(Shader&& o) noexcept;

    void loadFromFiles(const std::string& vertPath, const std::string& fragPath);
    void use() const;

    void setInt(const char* name, int val)            const;
    void setFloat(const char* name, float val)        const;
    void setVec3(const char* name, const glm::vec3& v) const;
    void setVec4(const char* name, const glm::vec4& v) const;
    void setMat4(const char* name, const glm::mat4& m) const;

    [[nodiscard]] GLuint id() const { return m_program; }

private:
    GLuint m_program = 0;

    static GLuint compileShader(GLenum type, const std::string& source);
    static std::string readFile(const std::string& path);
};

} // namespace mc
