#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "voxel_engine/callbacks.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/network/i_server_connection.h"
#include "voxel_engine/save_format.h"

// render distance in chunk nbrs
// TODO: maybe move this to config or even game
#define DEFAULT_RENDER_DISTANCE 16

namespace voxeng::server {

class SaveManager;

enum Direction {EAST, WEST, NORTH, SOUTH};

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
    const Chunk *tryGetChunk(ChunkID cid) const;

    void setSaveManager(SaveManager* save_manager);
    void flushAllDirtyChunks();

    World(network::IServerConnection& connection, const std::vector<VoxelType>& voxel_types, uint64_t seed, bool generate_chunks = true);
    ~World();

    World(World&&) noexcept;
    World& operator=(World&&) noexcept;

    void update();
    void setSeed(uint64_t seed);
    uint64_t getSeed() const;

    void updateChunks(WorldCoord pos);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}
