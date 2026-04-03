#include "core/Engine.h"
#include "world/Block.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <string>

namespace mc {

Engine::Engine()
    : m_window(makeWindowConfig())
{
    m_window.captureMouse(true);
    m_camera.position = glm::vec3(0.0f, 100.0f, 0.0f);

    initShaders();
    initTextures();

    m_world.seed = 12345;
    m_world.renderDistance = 8;
}

WindowConfig Engine::makeWindowConfig() {
    WindowConfig cfg;
    cfg.title  = "Minecraft Clone - Phase 2";
    cfg.width  = 1600;
    cfg.height = 900;
    cfg.vsync  = false;
    return cfg;
}

void Engine::initShaders() {
    m_chunkShader.loadFromFiles("assets/shaders/chunk.vert",
                                "assets/shaders/chunk.frag");
}

void Engine::initTextures() {
    m_blockTextures.create(16, Block::Tex::LAYER_COUNT);

    auto tryLoad = [&](int layer, const char* name,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        std::string base(name);
        std::string paths[] = {
            "assets/textures/block/" + base,
            "assets/textures/" + base,
        };
        for (auto& p : paths) {
            if (m_blockTextures.loadLayerAuto(layer, p)) {
                std::printf("  Loaded: %s\n", p.c_str());
                return;
            }
        }
        std::printf("  Fallback colour for: %s\n", name);
        m_blockTextures.fillSolid(layer, r, g, b, a);
    };

    std::printf("Loading textures...\n");
    tryLoad(Block::Tex::Stone,                "stone.png",                    128, 128, 128);
    tryLoad(Block::Tex::Dirt,                 "dirt.png",                     134,  96,  67);
    tryLoad(Block::Tex::GrassBlockSide,       "grass_block_side.png",         100, 120,  67);
    tryLoad(Block::Tex::GrassBlockTop,        "grass_block_top.png",           86, 168,  57);
    tryLoad(Block::Tex::Sand,                 "sand.png",                     219, 211, 160);
    tryLoad(Block::Tex::WaterStill,           "water_still.png",               30,  90, 200, 160);
    tryLoad(Block::Tex::Snow,                 "snow.png",                     240, 240, 255);
    tryLoad(Block::Tex::GrassBlockSideSnowed, "grass_block_side_snowed.png",  200, 210, 220);
    tryLoad(Block::Tex::Gravel,               "gravel.png",                   140, 130, 120);
    tryLoad(Block::Tex::Bedrock,              "bedrock.png",                   50,  50,  50);
    tryLoad(Block::Tex::CoalOre,              "coal_ore.png",                  70,  70,  70);
    tryLoad(Block::Tex::IronOre,              "iron_ore.png",                 160, 140, 120);
    std::printf("Textures loaded.\n");

    m_blockTextures.generateMipmaps();
}

void Engine::run() {
    double lastTime = glfwGetTime();

    while (!m_window.shouldClose()) {
        double now = glfwGetTime();
        float  dt  = static_cast<float>(now - lastTime);
        lastTime   = now;

        m_window.pollEvents();
        processGlobalInput();

        double mx, my;
        m_window.getMouseDelta(mx, my);
        m_camera.processMouseMovement(mx, my);
        m_camera.processInput(m_window, dt);

        m_world.update(m_camera.position);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        renderWorld();
        glDisable(GL_BLEND);
        updateTitle(dt);

        m_window.swapBuffers();
    }
}

void Engine::processGlobalInput() {
    if (m_window.isKeyDown(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(m_window.handle(), GLFW_TRUE);

    static bool f1Prev = false;
    bool f1Now = m_window.isKeyDown(GLFW_KEY_F1);
    if (f1Now && !f1Prev) {
        m_wireframe = !m_wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);
    }
    f1Prev = f1Now;

    static bool f2Prev = false;
    static bool mouseCaptured = true;
    bool f2Now = m_window.isKeyDown(GLFW_KEY_F2);
    if (f2Now && !f2Prev) {
        mouseCaptured = !mouseCaptured;
        m_window.captureMouse(mouseCaptured);
    }
    f2Prev = f2Now;
}

void Engine::renderWorld() {
    m_chunkShader.use();

    glm::mat4 view = m_camera.viewMatrix();
    glm::mat4 proj = m_camera.projectionMatrix(m_window.aspectRatio());
    glm::mat4 vp   = proj * view;

    m_chunkShader.setMat4("uViewProj", vp);
    m_chunkShader.setVec3("uCameraPos", m_camera.position);
    m_chunkShader.setFloat("uFogStart",
        static_cast<float>((m_world.renderDistance - 2) * CHUNK_X));
    m_chunkShader.setFloat("uFogEnd",
        static_cast<float>(m_world.renderDistance * CHUNK_X));
    m_chunkShader.setVec3("uFogColor", glm::vec3(0.53f, 0.81f, 0.92f));
    m_chunkShader.setInt("uTexArray", 0);

    m_blockTextures.bind(0);

    m_world.renderAll(m_chunkShader, vp);
}

void Engine::updateTitle(float dt) {
    static float timer  = 0.0f;
    static int   frames = 0;
    timer += dt;
    ++frames;
    if (timer >= 0.5f) {
        float fps = static_cast<float>(frames) / timer;
        char buf[256];
        std::snprintf(buf, sizeof(buf),
            "Minecraft Clone | FPS: %.0f | Chunks: %d | Pos: (%.0f, %.0f, %.0f)",
            fps, m_world.loadedChunkCount(),
            m_camera.position.x, m_camera.position.y, m_camera.position.z);
        m_window.setTitle(buf);
        timer  = 0.0f;
        frames = 0;
    }
}

} // namespace mc
