#include "core/Window.h"
#include <stdexcept>
#include <cstdio>

namespace mc {

Window::Window(const WindowConfig& cfg)
    : m_width(cfg.width), m_height(cfg.height)
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialise GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* monitor = cfg.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_window = glfwCreateWindow(m_width, m_height, cfg.title.c_str(), monitor, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(cfg.vsync ? 1 : 0);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("Failed to initialise GLAD");

    std::printf("OpenGL %s | %s | %s\n",
        glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));

    // Store `this` so static callbacks can reach the instance
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // light sky blue
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

Window::Window(Window&& o) noexcept
    : m_window(o.m_window), m_width(o.m_width), m_height(o.m_height),
      m_lastMouseX(o.m_lastMouseX), m_lastMouseY(o.m_lastMouseY),
      m_mouseDX(o.m_mouseDX), m_mouseDY(o.m_mouseDY),
      m_firstMouse(o.m_firstMouse)
{
    o.m_window = nullptr;
    if (m_window) glfwSetWindowUserPointer(m_window, this);
}

Window& Window::operator=(Window&& o) noexcept {
    if (this != &o) {
        if (m_window) {
            glfwDestroyWindow(m_window);
            glfwTerminate();
        }
        m_window = o.m_window; m_width = o.m_width; m_height = o.m_height;
        m_lastMouseX = o.m_lastMouseX; m_lastMouseY = o.m_lastMouseY;
        m_mouseDX = o.m_mouseDX; m_mouseDY = o.m_mouseDY;
        m_firstMouse = o.m_firstMouse;
        o.m_window = nullptr;
        if (m_window) glfwSetWindowUserPointer(m_window, this);
    }
    return *this;
}

bool Window::shouldClose() const { return glfwWindowShouldClose(m_window); }
void Window::pollEvents()        { glfwPollEvents(); }
void Window::swapBuffers()       { glfwSwapBuffers(m_window); }

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}

void Window::captureMouse(bool capture) {
    glfwSetInputMode(m_window, GLFW_CURSOR,
                     capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    m_firstMouse = true;
}

bool Window::isKeyDown(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonDown(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Window::getMouseDelta(double& dx, double& dy) {
    dx = m_mouseDX;
    dy = m_mouseDY;
    m_mouseDX = 0.0;
    m_mouseDY = 0.0;
}

// ── Static callbacks ──────────────────────────────────────────

void Window::framebufferSizeCallback(GLFWwindow* win, int w, int h) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
    self->m_width  = w;
    self->m_height = h;
    glViewport(0, 0, w, h);
}

void Window::cursorPosCallback(GLFWwindow* win, double x, double y) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
    if (self->m_firstMouse) {
        self->m_lastMouseX = x;
        self->m_lastMouseY = y;
        self->m_firstMouse = false;
        return;
    }
    self->m_mouseDX += x - self->m_lastMouseX;
    self->m_mouseDY += self->m_lastMouseY - y; // inverted Y
    self->m_lastMouseX = x;
    self->m_lastMouseY = y;
}

} // namespace mc
