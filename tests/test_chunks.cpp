#include <gtest/gtest.h>
#include <vector>

#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"

TEST(ChunkTest, BasicChunkTest) {
    Chunk c(DEFAULT_VOXEL_TYPES, {0.0f, 0.0f, 0.0f});
    EXPECT_EQ(c.getRendererId(), 0);
}
