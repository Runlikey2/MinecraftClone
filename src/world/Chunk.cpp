#include "world/Chunk.h"
#include <cstring>
#include <utility>

namespace mc {

Chunk::Chunk(glm::ivec2 chunkCoord) : coord(chunkCoord) {
    blocks.fill(Block::Air);
}

Chunk::~Chunk() { destroyGPU(); }

Chunk::Chunk(Chunk&& o) noexcept
    : coord(o.coord), blocks(std::move(o.blocks)),
      m_vao(o.m_vao), m_vbo(o.m_vbo), m_ebo(o.m_ebo),
      m_indexCount(o.m_indexCount), m_dirty(o.m_dirty)
{
    o.m_vao = o.m_vbo = o.m_ebo = 0;
    o.m_indexCount = 0;
}

Chunk& Chunk::operator=(Chunk&& o) noexcept {
    if (this != &o) {
        destroyGPU();
        coord = o.coord;
        blocks = std::move(o.blocks);
        m_vao = o.m_vao; m_vbo = o.m_vbo; m_ebo = o.m_ebo;
        m_indexCount = o.m_indexCount; m_dirty = o.m_dirty;
        o.m_vao = o.m_vbo = o.m_ebo = 0;
        o.m_indexCount = 0;
    }
    return *this;
}

BlockID Chunk::getBlock(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return Block::Air;
    return blocks[index(x, y, z)];
}

void Chunk::setBlock(int x, int y, int z, BlockID block) {
    if (!inBounds(x, y, z)) return;
    blocks[index(x, y, z)] = block;
    m_dirty = true;
}

void Chunk::uploadMesh(const std::vector<ChunkVertex>& vertices,
                       const std::vector<uint32_t>& indices)
{
    if (vertices.empty() || indices.empty()) {
        destroyGPU();
        m_indexCount = 0;
        return;
    }

    if (!m_vao) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);
    }

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(ChunkVertex)),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                 indices.data(), GL_STATIC_DRAW);

    // layout(location = 0) vec3 aPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex),
                          reinterpret_cast<void*>(offsetof(ChunkVertex, x)));

    // layout(location = 1) vec2 aUV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex),
                          reinterpret_cast<void*>(offsetof(ChunkVertex, u)));

    // layout(location = 2) float aTexLayer
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex),
                          reinterpret_cast<void*>(offsetof(ChunkVertex, texLayer)));

    // layout(location = 3) float aNormal
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex),
                          reinterpret_cast<void*>(offsetof(ChunkVertex, normalIndex)));

    glBindVertexArray(0);

    m_indexCount = static_cast<uint32_t>(indices.size());
}

void Chunk::render() const {
    if (m_indexCount == 0 || !m_vao) return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Chunk::destroyGPU() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);      m_vbo = 0; }
    if (m_ebo) { glDeleteBuffers(1, &m_ebo);      m_ebo = 0; }
}

} // namespace mc
