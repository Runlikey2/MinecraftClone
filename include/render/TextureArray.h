#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>

namespace mc {

class TextureArray {
public:
    TextureArray() = default;
    ~TextureArray();

    TextureArray(const TextureArray&) = delete;
    TextureArray& operator=(const TextureArray&) = delete;

    void create(int tileSize, int layerCount);

    bool loadLayer(int layer, const std::string& path);

    bool loadLayerCropped(int layer, const std::string& path);

    bool loadLayerAuto(int layer, const std::string& path);

    void fillSolid(int layer, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void fillCheckerboard(int layer,
                          uint8_t r1, uint8_t g1, uint8_t b1,
                          uint8_t r2, uint8_t g2, uint8_t b2,
                          int gridSize = 4);
    void generateMipmaps();
    void bind(GLuint unit = 0) const;

    [[nodiscard]] GLuint id()       const { return m_texture; }
    [[nodiscard]] int    tileSize() const { return m_tileSize; }
    [[nodiscard]] int    layers()   const { return m_layers; }

private:
    GLuint m_texture  = 0;
    int    m_tileSize = 0;
    int    m_layers   = 0;
};

} // namespace mc
