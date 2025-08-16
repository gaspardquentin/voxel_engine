#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "voxel_engine/math_utils.h"
#include "voxel_engine/voxel_types.h"
#include "../config.h"

using ChunkCoord = Vec3;
class Chunk{
public:
    static constexpr unsigned int SIZE = CHUNK_SIZE;
    static constexpr unsigned int WIDTH = CHUNK_WIDTH;
    static constexpr unsigned int HEIGHT = CHUNK_HEIGHT;
    static constexpr unsigned int DEPTH = CHUNK_DEPTH;
    const VoxelType& getVoxel(Vec3i pos) const;
    const VoxelType& getVoxelType(VoxelID vid) const;
    // Checks wether the chunk coord is not out of bounds 
    // (does not check if world coord is in this chunk)
    bool positionInChunk(Vec3i pos) const;
    inline void setRendererId(size_t id) { m_renderer_id = id; }
    inline size_t getRendererId() const { return m_renderer_id; }
    Chunk(const std::vector<VoxelType>& voxel_types);

private:
    // for renderer to be able to change vao/vbo only for updated chunks
    bool m_has_changed = true;
    size_t m_renderer_id = 0;

    const std::vector<VoxelType>& m_voxel_types;
    std::array<VoxelID, CHUNK_SIZE> m_data;
};
