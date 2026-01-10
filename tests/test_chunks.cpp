#include <gtest/gtest.h>
#include <vector>
#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"

class ChunkTest : public ::testing::Test {
protected:
    ChunkTest() : chunk(DEFAULT_VOXEL_TYPES, {10.0f, 20.0f, 30.0f}) {}
    Chunk chunk;
};

TEST_F(ChunkTest, Initialization) {
    EXPECT_EQ(chunk.getWorldPos().x, 10.0f);
    EXPECT_EQ(chunk.getWorldPos().y, 20.0f);
    EXPECT_EQ(chunk.getWorldPos().z, 30.0f);
}

TEST_F(ChunkTest, PositionInChunk) {
    EXPECT_TRUE(chunk.positionInChunk({0, 0, 0}));
    EXPECT_TRUE(chunk.positionInChunk({Chunk::WIDTH - 1, Chunk::HEIGHT - 1, Chunk::DEPTH - 1}));
    
    EXPECT_FALSE(chunk.positionInChunk({Chunk::WIDTH, 0, 0}));
    EXPECT_FALSE(chunk.positionInChunk({0, Chunk::HEIGHT, 0}));
    EXPECT_FALSE(chunk.positionInChunk({0, 0, Chunk::DEPTH}));
    
    EXPECT_FALSE(chunk.positionInChunk({static_cast<unsigned int>(-1), 0, 0}));
}

TEST_F(ChunkTest, WorldPositionInChunk) {
    Vec3f start = chunk.getWorldPos();
    EXPECT_TRUE(chunk.worldPositionInChunk(start));
    EXPECT_TRUE(chunk.worldPositionInChunk(start + Vec3f(0.5f, 0.5f, 0.5f)));
    EXPECT_TRUE(chunk.worldPositionInChunk(start + Vec3f(Chunk::WIDTH - 0.01f, Chunk::HEIGHT - 0.01f, Chunk::DEPTH - 0.01f)));

    EXPECT_FALSE(chunk.worldPositionInChunk(start + Vec3f(Chunk::WIDTH, 0, 0)));
    EXPECT_FALSE(chunk.worldPositionInChunk(start - Vec3f(0.1f, 0.0f, 0.0f)));
}

TEST_F(ChunkTest, GetChunkPosFromWorld) {
    Vec3f start = chunk.getWorldPos();
    
    ChunkCoord c1 = chunk.getChunkPosFromWorld(start);
    EXPECT_EQ(c1.x, 0);
    EXPECT_EQ(c1.y, 0);
    EXPECT_EQ(c1.z, 0);

    ChunkCoord c2 = chunk.getChunkPosFromWorld(start + Vec3f(5.5f, 5.5f, 5.5f));
    EXPECT_EQ(c2.x, 5);
    EXPECT_EQ(c2.y, 5);
    EXPECT_EQ(c2.z, 5);

    ChunkCoord c3 = chunk.getChunkPosFromWorld(start + Vec3f(Chunk::WIDTH - 1.0f, 0, 0));
    EXPECT_EQ(c3.x, Chunk::WIDTH - 1);
}

TEST_F(ChunkTest, SetAndGetVoxel) {
    ChunkCoord pos{5, 5, 5};
    VoxelID dirtId = 1; 

    chunk.setVoxel(pos, dirtId);
    const VoxelType& v = chunk.getVoxel(pos);
    EXPECT_EQ(v.getId(), dirtId);

    chunk.setVoxel(pos, 0);
    const VoxelType& v2 = chunk.getVoxel(pos);
    EXPECT_EQ(v2.getId(), 0);
}

TEST_F(ChunkTest, OutOfBoundsGetVoxelReturnsDefault) {
    ChunkCoord invalidPos{Chunk::WIDTH, 0, 0};
    const VoxelType& v = chunk.getVoxel(invalidPos);
    EXPECT_EQ(v.getId(), 0); 
}

TEST_F(ChunkTest, OutOfBoundsSetVoxelFails) {
    ChunkCoord invalidPos{Chunk::WIDTH, 0, 0};
    VoxelID dirtId = 1;
    VoxelID result = chunk.setVoxel(invalidPos, dirtId);
    EXPECT_EQ(result, 0); 
}