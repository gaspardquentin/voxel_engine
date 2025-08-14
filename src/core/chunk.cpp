#include "chunk.h"
#include <iostream>

bool Chunk::positionInChunk(Vec3i pos) const {
    size_t index = (pos.y * Chunk::DEPTH + pos.z) * Chunk::WIDTH + pos.x;
    return index < Chunk::SIZE;
}

const VoxelType& Chunk::getVoxel(Vec3i pos) const {
    size_t index = (pos.y * Chunk::DEPTH + pos.z) * Chunk::WIDTH + pos.x;
    if (!positionInChunk(pos)) {
        std::cerr << "<voxeng> WARNING: Voxel at " << pos << " out of Chunk bounds.\n";
        return m_voxel_types[0];
    }
    VoxelID vid = m_data[index];
    if (vid >= m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel at " << pos << " of invalid Voxel Type " << vid << ".\n";
        return m_voxel_types[0];
    }

    return m_voxel_types[m_data[index]];
}
const VoxelType& Chunk::getVoxelType(VoxelID vid) const {
    if (vid >= m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return m_voxel_types[0];
    }
    return m_voxel_types[vid];
}


Chunk::Chunk(const std::vector<VoxelType>& voxel_types): 
    m_has_changed(true),
    m_renderer_id(0),
    m_voxel_types(voxel_types) {
    // TODO: replace with better chunk generation

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {
                size_t index = (y * Chunk::DEPTH + z) * Chunk::WIDTH + x;
                if (y < CHUNK_HEIGHT/2) {
                    m_data[index] = 1; // dirt
                } else {
                    m_data[index] = 0; // air
                }
            }
        }
    }
}
