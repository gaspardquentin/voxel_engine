#include <gtest/gtest.h>
#include "voxel_engine/voxel_types.h"

using namespace voxeng;

TEST(VoxelTypes, AirProperties) {
    const auto& air = DEFAULT_VOXEL_TYPES[0];
    EXPECT_EQ(air.getId(), 0);
    EXPECT_EQ(air.getName(), "air");
    EXPECT_FALSE(air.isSolid());
    EXPECT_TRUE(air.isTransparent());
}

TEST(VoxelTypes, StoneProperties) {
    // Stone is id 3
    const auto& stone = DEFAULT_VOXEL_TYPES[3];
    EXPECT_EQ(stone.getName(), "stone");
    EXPECT_TRUE(stone.isSolid());
    EXPECT_FALSE(stone.isTransparent());
}

TEST(VoxelTypes, WaterProperties) {
    // Water is id 8
    const auto& water = DEFAULT_VOXEL_TYPES[8];
    EXPECT_EQ(water.getName(), "water");
    EXPECT_FALSE(water.isSolid());
    EXPECT_TRUE(water.isTransparent());
}

TEST(VoxelTypes, GlassProperties) {
    // Glass is id 10
    const auto& glass = DEFAULT_VOXEL_TYPES[10];
    EXPECT_EQ(glass.getName(), "glass");
    EXPECT_TRUE(glass.isSolid());
    EXPECT_TRUE(glass.isTransparent());
}

TEST(VoxelTypes, AllTypesHaveNames) {
    for (size_t i = 0; i < DEFAULT_VOXEL_TYPES.size(); i++) {
        EXPECT_FALSE(DEFAULT_VOXEL_TYPES[i].getName().empty())
            << "Voxel type at index " << i << " has empty name";
    }
}

TEST(VoxelTypes, IdsAreSequential) {
    for (size_t i = 0; i < DEFAULT_VOXEL_TYPES.size(); i++) {
        EXPECT_EQ(DEFAULT_VOXEL_TYPES[i].getId(), static_cast<VoxelID>(i))
            << "Voxel type at index " << i << " has unexpected id";
    }
}

TEST(VoxelTypes, AirIsIdZero) {
    // This is a contract: the engine assumes id 0 is always air
    ASSERT_FALSE(DEFAULT_VOXEL_TYPES.empty());
    EXPECT_EQ(DEFAULT_VOXEL_TYPES[0].getId(), 0);
    EXPECT_EQ(DEFAULT_VOXEL_TYPES[0].getName(), "air");
    EXPECT_FALSE(DEFAULT_VOXEL_TYPES[0].isSolid());
}
