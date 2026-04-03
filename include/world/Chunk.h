#pragma once

#include "world/Block.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>

namespace mc {

inline constexpr int CHUNK_X = 16;
inline constexpr int CHUNK_Y = 256;
inline constexpr int CHUNK_Z = 16;
inline constexpr int CHUNK_VOLUME = CHUNK_X * CHUNK_Y * CHUNK_Z;

/// Vertex sent to GPU for chunk rendering.
struct ChunkVertex {
    float x, y, z;       // local position within chunk
    float u, v;           // texture coordinates
    float texLayer;       // texture array layer index
    float normalIndex;    // 0–5 face direction (decoded in shader for lighting)
};

class Chunk {
public:
    glm::ivec2 coord;    // chunk coordinate (world_pos = coord * 16)

    explicit Chunk(glm::ivec2 chunkCoord);
    ~Chunk();

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) noexcept;
    Chunk& operator=(Chunk&&) noexcept;

    [[nodiscard]] BlockID getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockID block);

    void uploadMesh(const std::vector<ChunkVertex>& vertices,
                    const std::vector<uint32_t>& indices);
    void render() const;

    [[nodiscard]] bool hasMesh()  const { return m_indexCount > 0; }
    [[nodiscard]] bool isDirty()  const { return m_dirty; }
    void markDirty() { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    std::array<BlockID, CHUNK_VOLUME> blocks{};

private:
    GLuint   m_vao = 0, m_vbo = 0, m_ebo = 0;
    uint32_t m_indexCount = 0;
    bool     m_dirty = true;

    static int index(int x, int y, int z) {
        return y * (CHUNK_X * CHUNK_Z) + z * CHUNK_X + x;
    }
    static bool inBounds(int x, int y, int z) {
        return x >= 0 && x < CHUNK_X &&
               y >= 0 && y < CHUNK_Y &&
               z >= 0 && z < CHUNK_Z;
    }

    void destroyGPU();
};

} // namespace mc
