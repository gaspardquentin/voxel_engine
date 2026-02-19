#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>

#include "math_utils.h"
#include "voxel_types.h"

//TODO: better config approach (maybe chunk size in world attribute)
#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16
#define CHUNK_DEPTH 16 //TODO: are we really using it ?
#define CHUNK_SIZE (CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH)

using ChunkCoord = Vec3u;
class Chunk{
public:
    static constexpr unsigned int SIZE = CHUNK_SIZE;
    static constexpr unsigned int WIDTH = CHUNK_WIDTH;
    static constexpr unsigned int HEIGHT = CHUNK_HEIGHT;
    static constexpr unsigned int DEPTH = CHUNK_DEPTH;
    const VoxelType& getVoxel(ChunkCoord pos) const;
    VoxelID setVoxel(ChunkCoord pos, VoxelID new_voxel);
    const VoxelType& getVoxelType(VoxelID vid) const;
    const std::array<VoxelID, CHUNK_SIZE>& getRawData() const; // TODO: maybe remove this
    void setRawData(std::array<VoxelID, CHUNK_SIZE>&& new_data);
    // Checks wether the chunk coord is not out of bounds 
    // (does not check if world coord is in this chunk)
    bool positionInChunk(ChunkCoord pos) const;
    bool worldPositionInChunk(Vec3f world_pos) const;
    ChunkCoord getChunkPosFromWorld(Vec3f world_pos) const;
    const Vec3f& getWorldPos() const { return m_world_pos; }
    inline bool update() const {
        bool ret = m_has_changed;
        m_has_changed = false;
        return ret;
    }
    bool isPersistenceDirty() const { return m_persistence_dirty; }
    void clearPersistenceDirty() { m_persistence_dirty = false; }
    Chunk(const std::vector<VoxelType>& voxel_types, Vec3f position);
    Chunk(const std::vector<VoxelType>& voxel_types, Vec3f position, std::array<VoxelID, CHUNK_SIZE>&& raw_data);

private:
    // for renderer to be able to change vao/vbo only for updated chunks
    mutable bool m_has_changed = true;
    bool m_persistence_dirty = true;
    Vec3f m_world_pos; //TODO: note: maybe position y not needed (since the chunks take all the y-axis)

    const std::vector<VoxelType>& m_voxel_types;
    std::array<VoxelID, CHUNK_SIZE> m_data;
};
