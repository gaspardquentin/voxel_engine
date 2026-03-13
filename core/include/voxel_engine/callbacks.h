#pragma once

#include <functional>

#include "types.h"
#include "voxel_types.h"

/* TODO: redefine and implement callbacks once the engine works fine
using ChunkGeneratorCallback = std::function<void(ChunkCoord coord, VoxelID* chunkData)>;
using VoxelChangeCallback = std::function<void(WorldCoord pos, VoxelID oldVoxel, VoxelID newVoxel)>;


inline ChunkGeneratorCallback DEFAULT_CHUNK_GENERATOR = [](ChunkCoord coord, VoxelID* chunkData) {
    return;
};

inline VoxelChangeCallback DEFAULT_VOXEL_CHANGE_FUNC = [](WorldCoord pos, VoxelID oldVoxel, VoxelID newVoxel) {
    return;
};

*/
