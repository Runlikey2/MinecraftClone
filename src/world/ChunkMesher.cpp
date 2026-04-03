#include "world/ChunkMesher.h"
#include "world/Block.h"

namespace mc {

struct FaceDef {
    float verts[4][3];
    float uvs[4][2];
    int   nx, ny, nz;
};

static constexpr FaceDef s_faces[6] = {
    // +X (right)
    { {{1,0,1},{1,0,0},{1,1,0},{1,1,1}}, {{0,0},{1,0},{1,1},{0,1}},  1, 0, 0 },
    // -X (left)
    { {{0,0,0},{0,0,1},{0,1,1},{0,1,0}}, {{0,0},{1,0},{1,1},{0,1}}, -1, 0, 0 },
    // +Y (top)
    { {{0,1,1},{1,1,1},{1,1,0},{0,1,0}}, {{0,0},{1,0},{1,1},{0,1}},  0, 1, 0 },
    // -Y (bottom)
    { {{0,0,0},{1,0,0},{1,0,1},{0,0,1}}, {{0,0},{1,0},{1,1},{0,1}},  0,-1, 0 },
    // +Z (front)
    { {{0,0,1},{1,0,1},{1,1,1},{0,1,1}}, {{0,0},{1,0},{1,1},{0,1}},  0, 0, 1 },
    // -Z (back)
    { {{1,0,0},{0,0,0},{0,1,0},{1,1,0}}, {{0,0},{1,0},{1,1},{0,1}},  0, 0,-1 },
};

ChunkMesher::MeshData ChunkMesher::build(
    const Chunk& chunk,
    const Chunk* nPosX,
    const Chunk* nNegX,
    const Chunk* nPosZ,
    const Chunk* nNegZ)
{
    MeshData mesh;
    mesh.vertices.reserve(8192);
    mesh.indices.reserve(12288);

    auto getBlock = [&](int x, int y, int z) -> BlockID {
        if (y < 0 || y >= CHUNK_Y) return Block::Air;
        if (x < 0)        return nNegX ? nNegX->getBlock(CHUNK_X - 1, y, z) : Block::Air;
        if (x >= CHUNK_X) return nPosX ? nPosX->getBlock(0, y, z)           : Block::Air;
        if (z < 0)        return nNegZ ? nNegZ->getBlock(x, y, CHUNK_Z - 1) : Block::Air;
        if (z >= CHUNK_Z) return nPosZ ? nPosZ->getBlock(x, y, 0)           : Block::Air;
        return chunk.getBlock(x, y, z);
    };

    for (int y = 0; y < CHUNK_Y; ++y) {
        for (int z = 0; z < CHUNK_Z; ++z) {
            for (int x = 0; x < CHUNK_X; ++x) {
                BlockID block = chunk.getBlock(x, y, z);
                if (block == Block::Air) continue;

                bool blockIsOpaque = Block::isOpaque(block);

                for (int face = 0; face < 6; ++face) {
                    const auto& f = s_faces[face];
                    BlockID neighbor = getBlock(x + f.nx, y + f.ny, z + f.nz);

                    if (blockIsOpaque) {
                        if (Block::isOpaque(neighbor)) continue;
                    } else {
                        if (neighbor == block) continue;
                    }

                    int texLayer = Block::textureLayer(block, face);
                    uint32_t baseIdx = static_cast<uint32_t>(mesh.vertices.size());

                    for (int v = 0; v < 4; ++v) {
                        mesh.vertices.push_back({
                            static_cast<float>(x) + f.verts[v][0],
                            static_cast<float>(y) + f.verts[v][1],
                            static_cast<float>(z) + f.verts[v][2],
                            f.uvs[v][0],
                            f.uvs[v][1],
                            static_cast<float>(texLayer),
                            static_cast<float>(face)
                        });
                    }

                    mesh.indices.push_back(baseIdx + 0);
                    mesh.indices.push_back(baseIdx + 1);
                    mesh.indices.push_back(baseIdx + 2);
                    mesh.indices.push_back(baseIdx + 0);
                    mesh.indices.push_back(baseIdx + 2);
                    mesh.indices.push_back(baseIdx + 3);
                }
            }
        }
    }

    return mesh;
}

} // namespace mc
