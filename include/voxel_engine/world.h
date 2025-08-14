#pragma once

#include <vector>

#include "callbacks.h"


class World {
public:
    void setChunkGenerator(ChunkGeneratorCallback callback);
    void setVoxelChange(VoxelChangeCallback callback);

    // Important : the id 0 should imperatively be for AIR
    void setVoxelTypes(std::vector<VoxelType> voxel_types);
    World() {};

private:
    class Impl;
    Impl* m_impl;
};
