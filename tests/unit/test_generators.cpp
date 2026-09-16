#include <gtest/gtest.h>
#include "voxel_engine/server/chunk_generators.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"

using namespace voxeng;
using namespace voxeng::server;

// VoxelID constants matching DEFAULT_VOXEL_TYPES
constexpr VoxelID AIR   = 0;
constexpr VoxelID DIRT  = 2;
constexpr VoxelID STONE = 3;
constexpr VoxelID GRASS = 5;
constexpr VoxelID WATER = 8;

static VoxelID getVoxel(const std::array<VoxelID, CHUNK_SIZE>& data, unsigned x, unsigned y, unsigned z) {
    return data[Chunk::linearIndex({x, y, z})];
}

// =============================================================================
// FlatGenerator
// =============================================================================

TEST(FlatGenerator, LayerStructure) {
    FlatGenerator gen;
    auto data = gen.generate({0, 0});

    unsigned int ground = CHUNK_HEIGHT / 2;

    for (unsigned x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned z = 0; z < CHUNK_DEPTH; z++) {
            for (unsigned y = 0; y < CHUNK_HEIGHT; y++) {
                VoxelID v = getVoxel(data, x, y, z);
                if (y < ground - 2) {
                    EXPECT_EQ(v, STONE) << "Expected stone at y=" << y;
                } else if (y < ground) {
                    EXPECT_EQ(v, DIRT) << "Expected dirt at y=" << y;
                } else if (y == ground) {
                    EXPECT_EQ(v, GRASS) << "Expected grass at y=" << y;
                } else {
                    EXPECT_EQ(v, AIR) << "Expected air at y=" << y;
                }
            }
        }
    }
}

TEST(FlatGenerator, ConsistentAcrossChunks) {
    FlatGenerator gen;
    auto data1 = gen.generate({0, 0});
    auto data2 = gen.generate({5, 3});
    auto data3 = gen.generate({-10, -7});

    EXPECT_EQ(data1, data2);
    EXPECT_EQ(data2, data3);
}

TEST(FlatGenerator, NoSolidAboveSurface) {
    FlatGenerator gen;
    auto data = gen.generate({0, 0});
    unsigned int ground = CHUNK_HEIGHT / 2;

    for (unsigned x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned z = 0; z < CHUNK_DEPTH; z++) {
            for (unsigned y = ground + 1; y < CHUNK_HEIGHT; y++) {
                EXPECT_EQ(getVoxel(data, x, y, z), AIR)
                    << "Non-air block above surface at (" << x << "," << y << "," << z << ")";
            }
        }
    }
}

// =============================================================================
// PerlinGenerator
// =============================================================================

TEST(PerlinGenerator, Determinism) {
    PerlinGenerator gen(42);
    auto data1 = gen.generate({0, 0});
    auto data2 = gen.generate({0, 0});
    EXPECT_EQ(data1, data2);
}

TEST(PerlinGenerator, SeedSensitivity) {
    PerlinGenerator gen1(42);
    PerlinGenerator gen2(43);
    auto data1 = gen1.generate({0, 0});
    auto data2 = gen2.generate({0, 0});
    EXPECT_NE(data1, data2);
}

TEST(PerlinGenerator, LayerInvariants) {
    PerlinGenerator gen(42);
    auto data = gen.generate({0, 0});

    for (unsigned x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned z = 0; z < CHUNK_DEPTH; z++) {
            bool passed_surface = false;
            bool found_any_solid = false;

            for (unsigned y = 0; y < CHUNK_HEIGHT; y++) {
                VoxelID v = getVoxel(data, x, y, z);

                if (v == AIR && found_any_solid && !passed_surface) {
                    passed_surface = true;
                }

                // Once we pass the surface into air, no stone/dirt/grass should appear
                if (passed_surface && v != AIR && v != WATER) {
                    FAIL() << "Solid block " << (int)v << " above surface at ("
                           << x << "," << y << "," << z << ")";
                }

                if (v != AIR && v != WATER) {
                    found_any_solid = true;
                }
            }
        }
    }
}

TEST(PerlinGenerator, HeightBounds) {
    PerlinGenerator gen(42);
    auto data = gen.generate({0, 0});

    for (unsigned x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned z = 0; z < CHUNK_DEPTH; z++) {
            // Find the highest solid block
            int highest = -1;
            for (unsigned y = 0; y < CHUNK_HEIGHT; y++) {
                VoxelID v = getVoxel(data, x, y, z);
                if (v != AIR && v != WATER) {
                    highest = static_cast<int>(y);
                }
            }
            EXPECT_LT(highest, static_cast<int>(CHUNK_HEIGHT))
                << "Terrain overflows chunk at (" << x << "," << z << ")";
        }
    }
}

TEST(PerlinGenerator, NotAllAir) {
    PerlinGenerator gen(42);

    ChunkID test_chunks[] = {{0,0}, {1,0}, {0,1}, {-1,-1}, {5,3}, {-10,7}, {100,-50}, {0,-1}, {-3,3}, {42,42}};

    for (const auto& cid : test_chunks) {
        auto data = gen.generate(cid);
        bool has_solid = false;
        for (VoxelID v : data) {
            if (v != AIR) { has_solid = true; break; }
        }
        EXPECT_TRUE(has_solid) << "Chunk (" << cid.x << "," << cid.y << ") is 100% air";
    }
}

TEST(PerlinGenerator, NotAllSolid) {
    PerlinGenerator gen(42);

    ChunkID test_chunks[] = {{0,0}, {1,0}, {0,1}, {-1,-1}, {5,3}, {-10,7}, {100,-50}, {0,-1}, {-3,3}, {42,42}};

    for (const auto& cid : test_chunks) {
        auto data = gen.generate(cid);
        bool has_air = false;
        for (VoxelID v : data) {
            if (v == AIR) { has_air = true; break; }
        }
        EXPECT_TRUE(has_air) << "Chunk (" << cid.x << "," << cid.y << ") has no air";
    }
}
