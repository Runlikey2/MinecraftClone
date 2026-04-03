#pragma once

#include "world/Chunk.h"
#include <vector>

namespace mc {

class ChunkMesher {
public:
    struct MeshData {
        std::vector<ChunkVertex> vertices;
        std::vector<uint32_t>    indices;
    };

    static MeshData build(const Chunk& chunk,
                          const Chunk* nPosX = nullptr,
                          const Chunk* nNegX = nullptr,
                          const Chunk* nPosZ = nullptr,
                          const Chunk* nNegZ = nullptr);
};

} // namespace mc
