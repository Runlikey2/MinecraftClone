#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace mc {

struct WindowConfig {
    int         width    = 1600;
    int         height   = 900;
    std::string title    = "Minecraft Clone";
    bool        vsync    = false;
    bool        fullscreen = false;
};

class Window {
public:
    explicit Window(const WindowConfig& cfg = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& o) noexcept;
    Window& operator=(Window&& o) noexcept;

    [[nodiscard]] bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    void setTitle(const std::string& title);
    void captureMouse(bool capture);

    [[nodiscard]] GLFWwindow* handle() const { return m_window; }
    [[nodiscard]] int  width()  const { return m_width; }
    [[nodiscard]] int  height() const { return m_height; }
    [[nodiscard]] float aspectRatio() const {
        return m_height > 0 ? static_cast<float>(m_width) / m_height : 1.0f;
    }

    // Input queries
    [[nodiscard]] bool isKeyDown(int key) const;
    [[nodiscard]] bool isMouseButtonDown(int button) const;

    // Mouse delta (consumed on read)
    void getMouseDelta(double& dx, double& dy);

private:
    GLFWwindow* m_window = nullptr;
    int         m_width  = 0;
    int         m_height = 0;

    double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
    double m_mouseDX    = 0.0, m_mouseDY    = 0.0;
    bool   m_firstMouse = true;

    static void framebufferSizeCallback(GLFWwindow* win, int w, int h);
    static void cursorPosCallback(GLFWwindow* win, double x, double y);
};

} // namespace mc
