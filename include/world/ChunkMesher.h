#pragma once

#include "world/Chunk.h"
#include <vector>

namespace mc {

/// Stateless mesh builder — takes a chunk and optionally its 4 neighbors,
/// outputs vertices + indices with hidden-face culling.
class ChunkMesher {
public:
    struct MeshData {
        std::vector<ChunkVertex> vertices;
        std::vector<uint32_t>    indices;
    };

    /// Build mesh.  `neighbors` order: +X, -X, +Z, -Z  (may be nullptr).
    static MeshData build(const Chunk& chunk,
                          const Chunk* nPosX = nullptr,
                          const Chunk* nNegX = nullptr,
                          const Chunk* nPosZ = nullptr,
                          const Chunk* nNegZ = nullptr);
};

} // namespace mc
