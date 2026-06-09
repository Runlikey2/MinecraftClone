#include "world/World.h"
#include "render/Shader.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace mc {

World::World() {
    initNoise();
}

void World::setSeed(int s) {
    m_seed = s;
    initNoise();
}

void World::initNoise() {
    m_continentNoise.SetSeed(m_seed);
    m_continentNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_continentNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_continentNoise.SetFractalOctaves(6);
    m_continentNoise.SetFrequency(0.002f);

    m_erosionNoise.SetSeed(m_seed + 1);
    m_erosionNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_erosionNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_erosionNoise.SetFractalOctaves(4);
    m_erosionNoise.SetFrequency(0.008f);

    m_detailNoise.SetSeed(m_seed + 2);
    m_detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_detailNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_detailNoise.SetFractalOctaves(3);
    m_detailNoise.SetFrequency(0.03f);

    m_spagA.SetSeed(m_seed + 100);
    m_spagA.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_spagA.SetFrequency(0.02f);
    m_spagA.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_spagA.SetFractalOctaves(2);

    m_spagB.SetSeed(m_seed + 200);
    m_spagB.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    m_spagB.SetFrequency(0.02f);
    m_spagB.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_spagB.SetFractalOctaves(2);

    m_entranceNoise.SetSeed(m_seed + 500);
    m_entranceNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_entranceNoise.SetFrequency(0.01f);

    m_oreNoise.SetSeed(m_seed + 400);
    m_oreNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    m_oreNoise.SetFrequency(0.15f);
    m_oreNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
}

glm::ivec2 World::worldToChunkCoord(const glm::vec3& pos) {
    return {
        static_cast<int>(std::floor(pos.x / CHUNK_X)),
        static_cast<int>(std::floor(pos.z / CHUNK_Z))
    };
}

Chunk* World::getChunk(glm::ivec2 coord) {
    auto it = m_chunks.find(coord);
    return (it != m_chunks.end()) ? it->second.get() : nullptr;
}

void World::update(const glm::vec3& cameraPos) {
    glm::ivec2 center = worldToChunkCoord(cameraPos);
    int rd = renderDistance;

    int unloadDist = rd + 2;
    std::vector<glm::ivec2> toRemove;
    for (auto& [coord, chunk] : m_chunks) {
        int dx = std::abs(coord.x - center.x);
        int dz = std::abs(coord.y - center.y);
        if (dx > unloadDist || dz > unloadDist)
            toRemove.push_back(coord);
    }
    for (auto& c : toRemove) m_chunks.erase(c);

    constexpr int MAX_GEN_PER_FRAME = 4;
    int generated = 0;

    for (int ring = 0; ring <= rd && generated < MAX_GEN_PER_FRAME; ++ring) {
        for (int dx = -ring; dx <= ring && generated < MAX_GEN_PER_FRAME; ++dx) {
            for (int dz = -ring; dz <= ring && generated < MAX_GEN_PER_FRAME; ++dz) {
                if (ring > 0 && std::abs(dx) != ring && std::abs(dz) != ring)
                    continue;

                glm::ivec2 coord{center.x + dx, center.y + dz};
                if (m_chunks.contains(coord)) continue;

                auto chunk = std::make_unique<Chunk>(coord);
                generateTerrain(*chunk);
                m_chunks[coord] = std::move(chunk);
                ++generated;

                // Mark neighbors dirty for cross-boundary face culling
                static constexpr glm::ivec2 dirs[] = {{1,0},{-1,0},{0,1},{0,-1}};
                for (auto& d : dirs) {
                    if (auto* nb = getChunk(coord + d))
                        nb->markDirty();
                }
            }
        }
    }

    constexpr int MAX_MESH_PER_FRAME = 8;
    int meshed = 0;
    for (auto& [coord, chunk] : m_chunks) {
        if (chunk->isDirty() && meshed < MAX_MESH_PER_FRAME) {
            meshChunk(*chunk);
            chunk->clearDirty();
            ++meshed;
        }
    }
}

void World::renderAll(const Shader& shader, const glm::mat4& viewProj) {
    m_frustum.update(viewProj);
    m_chunksRendered = 0;
    m_chunksCulled   = 0;

    for (auto& [coord, chunk] : m_chunks) {
        if (!chunk->hasMesh()) continue;

        glm::vec3 minPt(coord.x * CHUNK_X,   0.0f, coord.y * CHUNK_Z);
        glm::vec3 maxPt(minPt.x + CHUNK_X, CHUNK_Y, minPt.z + CHUNK_Z);

        if (!m_frustum.testAABB(minPt, maxPt)) {
            ++m_chunksCulled;
            continue;
        }

        shader.setVec3("uChunkOffset", glm::vec3(minPt.x, 0.0f, minPt.z));
        chunk->render();
        ++m_chunksRendered;
    }
}

static constexpr int SEA_LEVEL = 62;

void World::generateTerrain(Chunk& chunk) {
    int worldX = chunk.coord.x * CHUNK_X;
    int worldZ = chunk.coord.y * CHUNK_Z;

    for (int lz = 0; lz < CHUNK_Z; ++lz) {
        for (int lx = 0; lx < CHUNK_X; ++lx) {
            float wx = static_cast<float>(worldX + lx);
            float wz = static_cast<float>(worldZ + lz);

            float continent = m_continentNoise.GetNoise(wx, wz);
            float erosion   = m_erosionNoise.GetNoise(wx, wz);
            float detail    = m_detailNoise.GetNoise(wx, wz);

            float erosionFactor = (1.0f - erosion) * 0.5f + 0.5f;
            int height = 64 + static_cast<int>(continent * 36.0f * erosionFactor)
                            + static_cast<int>(detail * 5.0f);
            height = std::clamp(height, 2, CHUNK_Y - 2);

            float entrance = m_entranceNoise.GetNoise(wx, wz);
            bool  nearEntrance = (entrance > 0.7f) && (height > SEA_LEVEL + 5);

            for (int y = 0; y < CHUNK_Y; ++y) {
                BlockID block = Block::Air;

                if (y == 0) {
                    block = Block::Bedrock;
                } else if (y < 5) {
                    float bdrk = m_detailNoise.GetNoise(wx * 4.0f, float(y) * 4.0f, wz * 4.0f);
                    block = (bdrk > 0.0f) ? Block::Bedrock : Block::Stone;
                } else if (y < height - 4) {
                    block = Block::Stone;
                } else if (y < height) {
                    block = Block::Dirt;
                } else if (y == height) {
                    if (height > 100)
                        block = Block::Snow;
                    else if (height <= SEA_LEVEL)
                        block = Block::Sand;
                    else
                        block = Block::Grass;
                }

                if (block == Block::Stone) {
                    float ore = m_oreNoise.GetNoise(wx, float(y), wz);
                    // Iron: y 5-32, rarer — check before coal so it isn't masked
                    if (y < 32 && y > 5 && ore < -0.92f)
                        block = Block::IronOre;
                    // Coal: y 5-48, uncommon
                    else if (y < 48 && y > 5 && ore < -0.88f)
                        block = Block::CoalOre;
                }

                if (y > 5 && y < height - 1 &&
                    block != Block::Air && block != Block::Bedrock)
                {
                    float wy = static_cast<float>(y);
                    float a = m_spagA.GetNoise(wx, wy, wz);
                    float b = m_spagB.GetNoise(wx, wy, wz);

                    float tunnelDist = a * a + b * b;

                    float depthRatio = 1.0f - (wy / static_cast<float>(height));
                    float threshold = 0.018f + depthRatio * 0.012f;

                    if (nearEntrance && y > height - 12 && y < height)
                        threshold *= 2.5f;

                    if (tunnelDist < threshold) {
                        block = Block::Air;

                        if (y + 1 < height) {
                            BlockID above = chunk.getBlock(lx, y + 1, lz);
                            if (above != Block::Air && above != Block::Bedrock)
                                chunk.setBlock(lx, y + 1, lz, Block::Air);
                        }
                        if (y + 2 < height) {
                            BlockID above2 = chunk.getBlock(lx, y + 2, lz);
                            if (above2 != Block::Air && above2 != Block::Bedrock)
                                chunk.setBlock(lx, y + 2, lz, Block::Air);
                        }
                    }
                }

                if (block == Block::Air && y <= SEA_LEVEL && y > 0)
                    block = Block::Water;

                chunk.setBlock(lx, y, lz, block);
            }

            // Beach / shoreline sand pass
            if (height <= SEA_LEVEL + 2 && height > SEA_LEVEL - 3) {
                BlockID surface = chunk.getBlock(lx, height, lz);
                if (surface == Block::Grass || surface == Block::Dirt) {
                    chunk.setBlock(lx, height, lz, Block::Sand);
                    for (int dy = 1; dy <= 3 && height - dy > 0; ++dy) {
                        if (chunk.getBlock(lx, height - dy, lz) == Block::Dirt)
                            chunk.setBlock(lx, height - dy, lz, Block::Sand);
                    }
                }
            }

            // Ocean / riverbed gravel — replace top layer of sand below sea floor
            if (height < SEA_LEVEL - 3) {
                if (chunk.getBlock(lx, height, lz) == Block::Sand)
                    chunk.setBlock(lx, height, lz, Block::Gravel);
            }
        }
    }
}

void World::meshChunk(Chunk& chunk) {
    const Chunk* nPosX = getChunk({chunk.coord.x + 1, chunk.coord.y});
    const Chunk* nNegX = getChunk({chunk.coord.x - 1, chunk.coord.y});
    const Chunk* nPosZ = getChunk({chunk.coord.x, chunk.coord.y + 1});
    const Chunk* nNegZ = getChunk({chunk.coord.x, chunk.coord.y - 1});

    auto mesh = ChunkMesher::build(chunk, nPosX, nNegX, nPosZ, nNegZ);
    chunk.uploadMesh(mesh.vertices, mesh.indices);
}

} // namespace mc
