#pragma once

#include <cstdint>
#include <array>

namespace mc {

using BlockID = uint8_t;

namespace Block {
    inline constexpr BlockID Air       = 0;
    inline constexpr BlockID Stone     = 1;
    inline constexpr BlockID Dirt      = 2;
    inline constexpr BlockID Grass     = 3;
    inline constexpr BlockID Sand      = 4;
    inline constexpr BlockID Water     = 5;
    inline constexpr BlockID Snow      = 6;
    inline constexpr BlockID Gravel    = 7;
    inline constexpr BlockID Bedrock   = 8;
    inline constexpr BlockID CoalOre   = 9;
    inline constexpr BlockID IronOre   = 10;
    inline constexpr BlockID COUNT     = 11;

    // ── Texture array layer indices ───────────────────────────
    // Filenames follow Minecraft resource pack convention:
    //   assets/textures/block/<name>.png
    namespace Tex {
        inline constexpr int Stone                = 0;  // stone.png
        inline constexpr int Dirt                 = 1;  // dirt.png
        inline constexpr int GrassBlockSide       = 2;  // grass_block_side.png
        inline constexpr int GrassBlockTop        = 3;  // grass_block_top.png
        inline constexpr int Sand                 = 4;  // sand.png
        inline constexpr int WaterStill           = 5;  // water_still.png
        inline constexpr int Snow                 = 6;  // snow.png
        inline constexpr int GrassBlockSideSnowed = 7;  // grass_block_side_snowed.png  (snowy grass)
        inline constexpr int Gravel               = 8;  // gravel.png
        inline constexpr int Bedrock              = 9;  // bedrock.png
        inline constexpr int CoalOre              = 10; // coal_ore.png
        inline constexpr int IronOre              = 11; // iron_ore.png
        inline constexpr int LAYER_COUNT          = 12;
    }

    struct FaceTextures { int side, top, bottom; };

    /// faceIndex: 0=+X  1=-X  2=+Y  3=-Y  4=+Z  5=-Z
    inline int textureLayer(BlockID block, int faceIndex) {
        static constexpr std::array<FaceTextures, COUNT> s_tex = {{
            /* Air     */ { 0, 0, 0 },
            /* Stone   */ { Tex::Stone,                Tex::Stone,           Tex::Stone },
            /* Dirt    */ { Tex::Dirt,                  Tex::Dirt,            Tex::Dirt },
            /* Grass   */ { Tex::GrassBlockSide,       Tex::GrassBlockTop,   Tex::Dirt },
            /* Sand    */ { Tex::Sand,                  Tex::Sand,            Tex::Sand },
            /* Water   */ { Tex::WaterStill,            Tex::WaterStill,      Tex::WaterStill },
            /* Snow    */ { Tex::GrassBlockSideSnowed,  Tex::Snow,            Tex::Dirt },
            /* Gravel  */ { Tex::Gravel,                Tex::Gravel,          Tex::Gravel },
            /* Bedrock */ { Tex::Bedrock,               Tex::Bedrock,         Tex::Bedrock },
            /* Coal    */ { Tex::CoalOre,               Tex::CoalOre,         Tex::CoalOre },
            /* Iron    */ { Tex::IronOre,               Tex::IronOre,         Tex::IronOre },
        }};

        if (block >= COUNT) return 0;
        const auto& ft = s_tex[block];
        if (faceIndex == 2) return ft.top;
        if (faceIndex == 3) return ft.bottom;
        return ft.side;
    }

    inline bool isOpaque(BlockID b) { return b != Air && b != Water; }
    inline bool isTransparent(BlockID b) { return b == Air || b == Water; }

} // namespace Block
} // namespace mc
