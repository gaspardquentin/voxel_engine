#include <gtest/gtest.h>
#include "voxel_engine/world_coords.h"
#include "voxel_engine/chunk.h"

using namespace voxeng;

// =============================================================================
// getChunkId — WorldCoord → ChunkID
// ChunkID.x = floor(pos.x / 16), ChunkID.y = floor(pos.z / 16)
// =============================================================================

TEST(GetChunkId, Origin) {
    ChunkID id = getChunkId({0.0f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, 0);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, InsideChunkZero) {
    ChunkID id = getChunkId({15.9f, 5.0f, 15.9f});
    EXPECT_EQ(id.x, 0);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, ExactlyOnNextBoundary_X) {
    ChunkID id = getChunkId({16.0f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, 1);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, EndOfChunkOne_X) {
    ChunkID id = getChunkId({31.9f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, 1);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, ChunkTwo_X) {
    ChunkID id = getChunkId({32.0f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, 2);
    EXPECT_EQ(id.y, 0);
}

// Critical negative coordinate tests
TEST(GetChunkId, JustPastZeroNegative_X) {
    ChunkID id = getChunkId({-0.1f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, -1);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, NegativeOne_X) {
    ChunkID id = getChunkId({-1.0f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, -1);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, ExactNegativeBoundary_X) {
    // -16.0 / 16 = -1.0, floor(-1.0) = -1
    ChunkID id = getChunkId({-16.0f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, -1);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, JustPastNegativeBoundary_X) {
    // -16.1 / 16 ≈ -1.006, floor = -2
    ChunkID id = getChunkId({-16.1f, 0.0f, 0.0f});
    EXPECT_EQ(id.x, -2);
    EXPECT_EQ(id.y, 0);
}

TEST(GetChunkId, NegativeZ) {
    ChunkID id = getChunkId({0.0f, 0.0f, -0.1f});
    EXPECT_EQ(id.x, 0);
    EXPECT_EQ(id.y, -1);
}

TEST(GetChunkId, BothAxesNegative) {
    ChunkID id = getChunkId({-0.1f, 0.0f, -0.1f});
    EXPECT_EQ(id.x, -1);
    EXPECT_EQ(id.y, -1);
}

TEST(GetChunkId, LargeCoordinates) {
    ChunkID id = getChunkId({1000.5f, 0.0f, -500.3f});
    EXPECT_EQ(id.x, 62);  // floor(1000.5/16) = floor(62.53) = 62
    EXPECT_EQ(id.y, -32); // floor(-500.3/16) = floor(-31.27) = -32
}

TEST(GetChunkId, YAxisIgnored) {
    // Y position should not affect chunk ID
    ChunkID id1 = getChunkId({5.0f, 0.0f, 5.0f});
    ChunkID id2 = getChunkId({5.0f, 100.0f, 5.0f});
    ChunkID id3 = getChunkId({5.0f, -50.0f, 5.0f});
    EXPECT_EQ(id1, id2);
    EXPECT_EQ(id2, id3);
}

// =============================================================================
// getChunkWorldPos — ChunkID → WorldCoord
// =============================================================================

TEST(GetChunkWorldPos, Origin) {
    Vec3f pos = getChunkWorldPos({0, 0});
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.z, 0.0f);
}

TEST(GetChunkWorldPos, PositiveX) {
    Vec3f pos = getChunkWorldPos({1, 0});
    EXPECT_FLOAT_EQ(pos.x, 16.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.z, 0.0f);
}

TEST(GetChunkWorldPos, PositiveZ) {
    Vec3f pos = getChunkWorldPos({0, 1});
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.z, 16.0f);
}

TEST(GetChunkWorldPos, Negative) {
    Vec3f pos = getChunkWorldPos({-1, -1});
    EXPECT_FLOAT_EQ(pos.x, -16.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.z, -16.0f);
}

// =============================================================================
// Roundtrip: getChunkWorldPos(getChunkId(pos)) == chunk origin
// =============================================================================

TEST(CoordinateRoundtrip, PositivePosition) {
    WorldCoord pos{5.5f, 3.0f, 10.2f};
    Vec3f origin = getChunkWorldPos(getChunkId(pos));
    // Origin should be (0, 0, 0) since pos is in chunk (0, 0)
    EXPECT_FLOAT_EQ(origin.x, 0.0f);
    EXPECT_FLOAT_EQ(origin.z, 0.0f);
}

TEST(CoordinateRoundtrip, NegativePosition) {
    WorldCoord pos{-5.0f, 0.0f, -10.0f};
    Vec3f origin = getChunkWorldPos(getChunkId(pos));
    // pos is in chunk (-1, -1), origin should be (-16, 0, -16)
    EXPECT_FLOAT_EQ(origin.x, -16.0f);
    EXPECT_FLOAT_EQ(origin.z, -16.0f);
}

// =============================================================================
// Chunk::linearIndex
// Formula: (y * DEPTH + z) * WIDTH + x
// =============================================================================

TEST(LinearIndex, Origin) {
    EXPECT_EQ(Chunk::linearIndex({0, 0, 0}), 0u);
}

TEST(LinearIndex, XAxis) {
    EXPECT_EQ(Chunk::linearIndex({1, 0, 0}), 1u);
}

TEST(LinearIndex, ZAxis) {
    EXPECT_EQ(Chunk::linearIndex({0, 0, 1}), 16u); // CHUNK_WIDTH
}

TEST(LinearIndex, YAxis) {
    EXPECT_EQ(Chunk::linearIndex({0, 1, 0}), 256u); // CHUNK_WIDTH * CHUNK_DEPTH
}

TEST(LinearIndex, MaxCorner) {
    EXPECT_EQ(Chunk::linearIndex({15, 15, 15}), CHUNK_SIZE - 1);
}

// =============================================================================
// ChunkID hash — different IDs should produce different hashes
// =============================================================================

TEST(ChunkIdHash, DifferentIds) {
    std::hash<ChunkID> hasher;
    EXPECT_NE(hasher({0, 0}), hasher({1, 0}));
    EXPECT_NE(hasher({0, 0}), hasher({0, 1}));
}

TEST(ChunkIdHash, AsymmetricIds) {
    std::hash<ChunkID> hasher;
    EXPECT_NE(hasher({1, 2}), hasher({2, 1}));
}
