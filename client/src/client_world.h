#pragma once

#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/types.h"
#include "voxel_engine/voxel_types.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace voxeng::client {

struct RenderEntity {
    //EntityID entity_id; TODO: needed ?
    std::string model_name;
    WorldCoord position;
    float yaw = 0.f;
    float pitch = 0.f;
    bool update_dirty = true; //TODO: maybe not needed but still
};


class ClientWorld {
    std::vector<VoxelType> m_voxel_types;
    std::unordered_map<ChunkID, Chunk> m_chunks;
    std::unordered_map<EntityID, RenderEntity> m_entities;
    const uint8_t& m_render_distance;

public:
    ClientWorld(std::vector<VoxelType> voxel_types, const uint8_t& render_distance);

    void loadChunk(ChunkID id, std::array<VoxelID, Chunk::SIZE>&& data);
    void unloadChunk(ChunkID id);

    void spawnEntity(EntityID id, RenderEntity entity);
    void despawnEntity(EntityID id);
    RenderEntity *getEntity(EntityID id);

    const std::unordered_map<ChunkID, Chunk>& getChunks() const;
    const std::unordered_map<EntityID, RenderEntity>& getEntities() const;

    VoxelID setVoxel(ChunkID chunk_id, ChunkCoord chunk_pos, VoxelID new_voxel);
    const VoxelType& getVoxel(WorldCoord pos) const;
    const VoxelType& getVoxelType(VoxelID vid) const;
    const std::vector<VoxelType>& getVoxelTypes() const;
};

}
