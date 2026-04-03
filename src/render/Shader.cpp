#include "render/Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdio>

namespace mc {

Shader::~Shader() {
    if (m_program) glDeleteProgram(m_program);
}

Shader::Shader(Shader&& o) noexcept : m_program(o.m_program) { o.m_program = 0; }
Shader& Shader::operator=(Shader&& o) noexcept {
    if (this != &o) {
        if (m_program) glDeleteProgram(m_program);
        m_program = o.m_program;
        o.m_program = 0;
    }
    return *this;
}

void Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath) {
    std::string vSrc = readFile(vertPath);
    std::string fSrc = readFile(fragPath);

    GLuint vs = compileShader(GL_VERTEX_SHADER,   vSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fSrc);

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint ok = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        glDeleteProgram(m_program);
        m_program = 0;
        throw std::runtime_error(std::string("Shader link error: ") + log);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shader::use() const { glUseProgram(m_program); }

void Shader::setInt(const char* n, int v)              const { glUniform1i(glGetUniformLocation(m_program, n), v); }
void Shader::setFloat(const char* n, float v)          const { glUniform1f(glGetUniformLocation(m_program, n), v); }
void Shader::setVec3(const char* n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(m_program, n), 1, glm::value_ptr(v)); }
void Shader::setVec4(const char* n, const glm::vec4& v) const { glUniform4fv(glGetUniformLocation(m_program, n), 1, glm::value_ptr(v)); }
void Shader::setMat4(const char* n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(m_program, n), 1, GL_FALSE, glm::value_ptr(m)); }

GLuint Shader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        const char* label = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        glDeleteShader(shader);
        throw std::runtime_error(std::string(label) + " shader compile error: " + log);
    }
    return shader;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open shader file: " + path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // namespace mc
