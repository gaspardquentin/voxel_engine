#include <gtest/gtest.h>
#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"

using namespace voxeng;

// Fixture: creates a chunk at origin filled with air (all zeros)
class ChunkTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::array<VoxelID, CHUNK_SIZE> raw{};
        chunk = std::make_unique<Chunk>(DEFAULT_VOXEL_TYPES, Vec3f{0.0f, 0.0f, 0.0f}, std::move(raw));
    }

    std::unique_ptr<Chunk> chunk;
};

// ===== Set & Get =====

TEST_F(ChunkTest, SetAndGetVoxel) {
    ChunkCoord pos{5, 5, 5};
    chunk->setVoxel(pos, 2); // Dirt
    EXPECT_EQ(chunk->getVoxel(pos).getId(), 2);

    chunk->setVoxel(pos, 0); // Back to air
    EXPECT_EQ(chunk->getVoxel(pos).getId(), 0);
}

TEST_F(ChunkTest, SetVoxelReturnsOldValue) {
    ChunkCoord pos{3, 3, 3};
    chunk->setVoxel(pos, 3); // Stone
    VoxelID old = chunk->setVoxel(pos, 2); // Replace with dirt
    EXPECT_EQ(old, 3);
}

// ===== Bounds Checking =====

TEST_F(ChunkTest, PositionInChunk_ValidPositions) {
    EXPECT_TRUE(chunk->positionInChunk({0, 0, 0}));
    EXPECT_TRUE(chunk->positionInChunk({15, 15, 15}));
    EXPECT_TRUE(chunk->positionInChunk({8, 8, 8}));
}

TEST_F(ChunkTest, PositionInChunk_InvalidPositions) {
    EXPECT_FALSE(chunk->positionInChunk({16, 0, 0}));
    EXPECT_FALSE(chunk->positionInChunk({0, 16, 0}));
    EXPECT_FALSE(chunk->positionInChunk({0, 0, 16}));
}

TEST_F(ChunkTest, PositionInChunk_UnsignedWraparound) {
    // static_cast<unsigned>(-1) is a huge number, should be out of bounds
    ChunkCoord bad{static_cast<unsigned int>(-1), 0, 0};
    EXPECT_FALSE(chunk->positionInChunk(bad));
}

TEST_F(ChunkTest, OutOfBoundsGetVoxel) {
    // Should return air (id 0) for out-of-bounds
    const VoxelType& v = chunk->getVoxel({16, 0, 0});
    EXPECT_EQ(v.getId(), 0);
}

TEST_F(ChunkTest, OutOfBoundsSetVoxel) {
    VoxelID result = chunk->setVoxel({16, 0, 0}, 2);
    EXPECT_EQ(result, 0);
}

// ===== World Position In Chunk =====

TEST_F(ChunkTest, WorldPositionInChunk_Inside) {
    EXPECT_TRUE(chunk->worldPositionInChunk({0.0f, 0.0f, 0.0f}));
    EXPECT_TRUE(chunk->worldPositionInChunk({15.9f, 15.9f, 15.9f}));
    EXPECT_TRUE(chunk->worldPositionInChunk({8.0f, 8.0f, 8.0f}));
}

TEST_F(ChunkTest, WorldPositionInChunk_Outside) {
    EXPECT_FALSE(chunk->worldPositionInChunk({16.0f, 0.0f, 0.0f}));
    EXPECT_FALSE(chunk->worldPositionInChunk({-0.1f, 0.0f, 0.0f}));
    EXPECT_FALSE(chunk->worldPositionInChunk({0.0f, 16.0f, 0.0f}));
}

TEST_F(ChunkTest, WorldPositionInChunk_OffsetChunk) {
    std::array<VoxelID, CHUNK_SIZE> raw{};
    Chunk offset_chunk(DEFAULT_VOXEL_TYPES, Vec3f{32.0f, 0.0f, 16.0f}, std::move(raw));

    EXPECT_TRUE(offset_chunk.worldPositionInChunk({32.0f, 0.0f, 16.0f}));
    EXPECT_TRUE(offset_chunk.worldPositionInChunk({47.9f, 15.9f, 31.9f}));
    EXPECT_FALSE(offset_chunk.worldPositionInChunk({48.0f, 0.0f, 16.0f}));
    EXPECT_FALSE(offset_chunk.worldPositionInChunk({31.9f, 0.0f, 16.0f}));
}

// ===== Chunk Pos From World =====

TEST_F(ChunkTest, GetChunkPosFromWorld) {
    ChunkCoord pos = chunk->getChunkPosFromWorld({5.5f, 3.0f, 7.9f});
    EXPECT_EQ(pos.x, 5u);
    EXPECT_EQ(pos.y, 3u);
    EXPECT_EQ(pos.z, 7u);
}

TEST_F(ChunkTest, GetChunkPosFromWorld_Origin) {
    ChunkCoord pos = chunk->getChunkPosFromWorld({0.0f, 0.0f, 0.0f});
    EXPECT_EQ(pos.x, 0u);
    EXPECT_EQ(pos.y, 0u);
    EXPECT_EQ(pos.z, 0u);
}

// ===== Dirty Flags =====

TEST_F(ChunkTest, DirtyFlags_InitialState) {
    // Chunk created from raw data: persistence dirty is false
    EXPECT_FALSE(chunk->isPersistenceDirty());
    // Render dirty is always true on construction
    EXPECT_TRUE(chunk->isRenderDirty());
}

TEST_F(ChunkTest, DirtyFlags_AfterSetVoxel) {
    chunk->clearRenderDirty();
    EXPECT_FALSE(chunk->isRenderDirty());

    chunk->setVoxel({0, 0, 0}, 3);
    EXPECT_TRUE(chunk->isRenderDirty());
    EXPECT_TRUE(chunk->isPersistenceDirty());
}

TEST_F(ChunkTest, DirtyFlags_ClearWorks) {
    chunk->setVoxel({0, 0, 0}, 3);
    chunk->clearRenderDirty();
    chunk->clearPersistenceDirty();
    EXPECT_FALSE(chunk->isRenderDirty());
    EXPECT_FALSE(chunk->isPersistenceDirty());
}

// ===== Raw Data Access =====

TEST_F(ChunkTest, GetRawData_MatchesSetVoxel) {
    chunk->setVoxel({3, 7, 11}, 5); // Grass
    const auto& raw = chunk->getRawData();
    EXPECT_EQ(raw[Chunk::linearIndex({3, 7, 11})], 5);
}

TEST_F(ChunkTest, LinearIndex_Consistency) {
    // Set voxels at multiple positions, verify via raw data
    struct TestCase { ChunkCoord pos; VoxelID id; };
    std::vector<TestCase> cases = {
        {{0, 0, 0}, 1},
        {{15, 0, 0}, 2},
        {{0, 15, 0}, 3},
        {{0, 0, 15}, 4},
        {{7, 8, 9}, 5},
    };

    for (const auto& tc : cases) {
        chunk->setVoxel(tc.pos, tc.id);
    }

    const auto& raw = chunk->getRawData();
    for (const auto& tc : cases) {
        EXPECT_EQ(raw[Chunk::linearIndex(tc.pos)], tc.id)
            << "Mismatch at (" << tc.pos.x << "," << tc.pos.y << "," << tc.pos.z << ")";
    }
}
