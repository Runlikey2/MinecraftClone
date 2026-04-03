#include "render/TextureArray.h"
#include <stb_image.h>
#include <vector>
#include <cstdio>

namespace mc {

TextureArray::~TextureArray() {
    if (m_texture) glDeleteTextures(1, &m_texture);
}

void TextureArray::create(int tileSize, int layerCount) {
    m_tileSize = tileSize;
    m_layers   = layerCount;

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8,
                 tileSize, tileSize, layerCount,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int maxLevel = 0;
    int s = tileSize;
    while (s > 1) { s >>= 1; maxLevel++; }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

bool TextureArray::loadLayer(int layer, const std::string& path) {
    stbi_set_flip_vertically_on_load(false);
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) return false;

    if (w != m_tileSize || h != m_tileSize) {
        stbi_image_free(data);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, layer,
                    m_tileSize, m_tileSize, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return true;
}

bool TextureArray::loadLayerCropped(int layer, const std::string& path) {
    stbi_set_flip_vertically_on_load(false);
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) return false;

    // Image must be at least tileSize wide and tall
    if (w < m_tileSize || h < m_tileSize) {
        stbi_image_free(data);
        return false;
    }

    // Extract the top-left tileSize × tileSize region (first frame)
    std::vector<uint8_t> cropped(m_tileSize * m_tileSize * 4);
    for (int row = 0; row < m_tileSize; ++row) {
        std::memcpy(
            cropped.data() + row * m_tileSize * 4,
            data + row * w * 4,
            m_tileSize * 4
        );
    }
    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, layer,
                    m_tileSize, m_tileSize, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, cropped.data());

    std::printf("  Loaded '%s' (cropped %dx%d -> %dx%d)\n",
                path.c_str(), w, h, m_tileSize, m_tileSize);
    return true;
}

bool TextureArray::loadLayerAuto(int layer, const std::string& path) {
    // Try exact match first
    if (loadLayer(layer, path)) return true;
    // Try cropped (handles animated strips like water_still.png 16x512)
    if (loadLayerCropped(layer, path)) return true;
    return false;
}

void TextureArray::fillSolid(int layer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    std::vector<uint8_t> pixels(m_tileSize * m_tileSize * 4);
    for (int i = 0; i < m_tileSize * m_tileSize; ++i) {
        pixels[i * 4 + 0] = r;
        pixels[i * 4 + 1] = g;
        pixels[i * 4 + 2] = b;
        pixels[i * 4 + 3] = a;
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, layer,
                    m_tileSize, m_tileSize, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void TextureArray::fillCheckerboard(int layer,
                                     uint8_t r1, uint8_t g1, uint8_t b1,
                                     uint8_t r2, uint8_t g2, uint8_t b2,
                                     int gridSize)
{
    std::vector<uint8_t> pixels(m_tileSize * m_tileSize * 4);
    for (int y = 0; y < m_tileSize; ++y) {
        for (int x = 0; x < m_tileSize; ++x) {
            bool even = ((x / gridSize) + (y / gridSize)) % 2 == 0;
            int idx = (y * m_tileSize + x) * 4;
            pixels[idx + 0] = even ? r1 : r2;
            pixels[idx + 1] = even ? g1 : g2;
            pixels[idx + 2] = even ? b1 : b2;
            pixels[idx + 3] = 255;
        }
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, layer,
                    m_tileSize, m_tileSize, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void TextureArray::generateMipmaps() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}

void TextureArray::bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture);
}

} // namespace mc
