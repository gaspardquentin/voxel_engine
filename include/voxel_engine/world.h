#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "callbacks.h"
#include "texture.h"
#include "voxel_types.h"


// render distance in chunk nbrs
#define DEFAULT_RENDER_DISTANCE 1//8

class World {
public:
    void setChunkGenerator(ChunkGeneratorCallback callback);
    void setVoxelChange(VoxelChangeCallback callback);

    // Important : the id 0 should imperatively be for AIR
    void setVoxelTypes(std::vector<VoxelType> voxel_types);

    const std::vector<VoxelType>& getVoxelTypes() const;
    const VoxelType& getVoxelType(VoxelID vid) const;

    // returns old voxel value
    VoxelID setVoxel(WorldCoord pos, VoxelID new_voxel);
    const VoxelType& getVoxel(WorldCoord pos) const;

    void setTextureForType(VoxelID vid, std::shared_ptr<Texture> texture);
    World(uint8_t render_distance);
    World();
    ~World();

    void update();
    void setRenderDistance(uint8_t render_distance);

private:
    class Impl;
    Impl* m_impl;
};
