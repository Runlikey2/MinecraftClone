#pragma once

#include "world/Chunk.h"
#include "world/ChunkMesher.h"
#include "core/Frustum.h"
#include <FastNoiseLite.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>

namespace mc {

class Shader;

struct IVec2Hash {
    std::size_t operator()(const glm::ivec2& v) const {
        auto h1 = std::hash<int>{}(v.x);
        auto h2 = std::hash<int>{}(v.y);
        return h1 ^ (h2 * 2654435761u);
    }
};

class World {
public:
    int renderDistance = 8;

    World();

    void setSeed(int s);
    [[nodiscard]] int seed() const { return m_seed; }

    void update(const glm::vec3& cameraPos);

    /// Render with frustum culling.  Pass the combined VP matrix.
    void renderAll(const Shader& shader, const glm::mat4& viewProj);

    [[nodiscard]] int loadedChunkCount()   const { return static_cast<int>(m_chunks.size()); }
    [[nodiscard]] int renderedChunkCount() const { return m_chunksRendered; }
    [[nodiscard]] int culledChunkCount()   const { return m_chunksCulled; }

    Chunk* getChunk(glm::ivec2 coord);

private:
    using ChunkMap = std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, IVec2Hash>;
    ChunkMap m_chunks;
    Frustum  m_frustum;
    int      m_seed          = 42;
    int      m_chunksRendered = 0;
    int      m_chunksCulled   = 0;

    // Noise generators — initialized once per seed change.
    FastNoiseLite m_continentNoise;
    FastNoiseLite m_erosionNoise;
    FastNoiseLite m_detailNoise;
    FastNoiseLite m_spagA;
    FastNoiseLite m_spagB;
    FastNoiseLite m_entranceNoise;
    FastNoiseLite m_oreNoise;

    void initNoise();
    void generateTerrain(Chunk& chunk);
    void meshChunk(Chunk& chunk);

    static glm::ivec2 worldToChunkCoord(const glm::vec3& pos);
};

} // namespace mc
