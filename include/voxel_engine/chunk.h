#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>

#include "math_utils.h"
#include "voxel_types.h"

//TODO: better config approach (maybe chunk size in world attribute)
#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16
#define CHUNK_DEPTH 16
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
    // Checks wether the chunk coord is not out of bounds 
    // (does not check if world coord is in this chunk)
    bool positionInChunk(ChunkCoord pos) const;
    bool worldPositionInChunk(Vec3f world_pos) const;
    ChunkCoord getChunkPosFromWorld(Vec3f world_pos) const;
    const Vec3f& getWorldPos() const { return m_world_pos; }
    inline void setRendererId(size_t id) { m_renderer_id = id; }
    inline size_t getRendererId() const { return m_renderer_id; }
    inline bool update() const {
        bool ret = m_has_changed;
        m_has_changed = false;
        return ret;
    }
    Chunk(const std::vector<VoxelType>& voxel_types, Vec3f position);

private:
    // for renderer to be able to change vao/vbo only for updated chunks
    mutable bool m_has_changed = true;
    size_t m_renderer_id = 0;
    Vec3f m_world_pos;

    const std::vector<VoxelType>& m_voxel_types;
    std::array<VoxelID, CHUNK_SIZE> m_data;
};
