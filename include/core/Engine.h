#pragma once

#include "core/Window.h"
#include "core/Camera.h"
#include "render/Shader.h"
#include "render/TextureArray.h"
#include "world/World.h"
#include <string>

namespace mc {

class Engine {
public:
    Engine();
    ~Engine() = default;

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void run();

private:
    Window       m_window;
    Camera       m_camera;
    Shader       m_chunkShader;
    TextureArray m_blockTextures;
    World        m_world;

    bool  m_wireframe = false;

    void initTextures();
    void initShaders();
    void processGlobalInput();
    void renderWorld();
    void updateTitle(float dt);

    static WindowConfig makeWindowConfig();
};

} // namespace mc
