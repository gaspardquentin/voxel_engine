#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "callbacks.h"
#include "texture.h"
#include "voxel_types.h"
#include "chunk.h"
#include "math_utils.h"
#include "save_format.h"

namespace voxeng { class SaveManager; }

//TODO: place this elsewhere
enum Direction {EAST, WEST, NORTH, SOUTH};

// render distance in chunk nbrs
// TODO: maybe move this to config or even game
#define DEFAULT_RENDER_DISTANCE 16

class World {
public:
    // Important : the id 0 should imperatively be for AIR
    void setVoxelTypes(std::vector<VoxelType> voxel_types);

    const std::vector<VoxelType>& getVoxelTypes() const;
    const VoxelType& getVoxelType(VoxelID vid) const;

    // returns old voxel value
    VoxelID setVoxel(WorldCoord pos, VoxelID new_voxel);
    const VoxelType& getVoxel(WorldCoord pos) const;

    const std::unordered_map<ChunkID, Chunk>& getChunks() const;

    void setTextureForType(VoxelID vid, std::shared_ptr<Texture> texture);

    void setSaveManager(voxeng::SaveManager* save_manager);
    void flushAllDirtyChunks();

    World(uint8_t render_distance, uint64_t seed, bool generate_chunks = true);
    World(uint64_t seed, bool generate_chunks = true);
    ~World();

    World(World&&) noexcept;
    World& operator=(World&&) noexcept;

    void update();
    void setRenderDistance(uint8_t render_distance);
    uint8_t getRenderDistance() const;
    void setSeed(uint64_t seed);
    uint64_t getSeed() const;

    void updateChunks(WorldCoord pos);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
