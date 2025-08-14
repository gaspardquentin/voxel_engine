#pragma once

#include <functional>
#include <glm/glm.hpp>

#include "voxel_types.h"
#include "math_utils.h"

using ChunkCoord = Vec3;
using WorldCoord = Vec3;

using ChunkGeneratorCallback = std::function<void(ChunkCoord coord, VoxelID* chunkData)>;
using VoxelChangeCallback = std::function<void(WorldCoord pos, VoxelID oldVoxel, VoxelID newVoxel)>;


inline ChunkGeneratorCallback DEFAULT_CHUNK_GENERATOR = [](ChunkCoord coord, VoxelID* chunkData) {
    return;
};

inline VoxelChangeCallback DEFAULT_VOXEL_CHANGE_FUNC = [](WorldCoord pos, VoxelID oldVoxel, VoxelID newVoxel) {
    return;
};
