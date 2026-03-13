
#include "client_world.h"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/world_coords.h"
#include <cstdint>

namespace voxeng::client {

ClientWorld::ClientWorld(std::vector<VoxelType> voxel_types, const uint8_t& render_distance):
    m_voxel_types(voxel_types), m_render_distance(render_distance) {}


void ClientWorld::loadChunk(ChunkID id, std::array<VoxelID, Chunk::SIZE>&& data) {
    WorldCoord pos = getChunkWorldPos(id);
    m_chunks.insert_or_assign(id, Chunk{m_voxel_types, pos, std::move(data)});
}

void ClientWorld::unloadChunk(ChunkID id) {
    m_chunks.erase(id);
}


//TODO: maybe add error handling here

void ClientWorld::spawnEntity(EntityID id, RenderEntity entity) {
    m_entities.insert({id, std::move(entity)});
}

void ClientWorld::despawnEntity(EntityID id) {
    m_entities.erase(id);
}

RenderEntity *ClientWorld::getEntity(EntityID id) {
    return &m_entities.at(id);
}

const std::unordered_map<EntityID, RenderEntity>& ClientWorld::getEntities() const {
    return m_entities;
}



const std::unordered_map<ChunkID, Chunk>& ClientWorld::getChunks() const {
    return m_chunks;
}

VoxelID ClientWorld::setVoxel(ChunkID chunk_id, ChunkCoord chunk_pos, VoxelID new_voxel) {
    auto pair = m_chunks.find(chunk_id);
    if (pair == m_chunks.end()) {
        std::cerr << "<voxeng> WARNING: Chunk " << chunk_id << " is not loaded in client.\n";
        return 0;
    }
    Chunk& chunk = pair->second;
    return chunk.setVoxel(chunk_pos, new_voxel);
}

const VoxelType& ClientWorld::getVoxel(WorldCoord pos) const {
    ChunkID chunk_id = voxeng::getChunkId(pos);
    auto pair = m_chunks.find(chunk_id);
    if (pair == m_chunks.end()) {
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        return m_voxel_types[0];
    }
    const Chunk& chunk = pair->second;
    ChunkCoord chunk_pos = chunk.getChunkPosFromWorld(pos);
    return chunk.getVoxel(chunk_pos);
}

const std::vector<VoxelType>& ClientWorld::getVoxelTypes() const {
    return m_voxel_types;
}

const VoxelType& ClientWorld::getVoxelType(VoxelID vid) const {
    if (vid >= m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return m_voxel_types[0];
    }
    return m_voxel_types[vid];
}

}

