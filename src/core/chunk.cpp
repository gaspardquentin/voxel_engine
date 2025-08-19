#include "chunk.h"
#include <iostream>

bool Chunk::positionInChunk(Vec3i pos) const {
    return  pos.x < Chunk::WIDTH &&
            pos.y < Chunk::HEIGHT &&
            pos.z < Chunk::DEPTH;
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


VoxelID Chunk::setVoxel(Vec3i pos, VoxelID new_voxel) {
    if (!positionInChunk(pos)) {
        std::cerr << "<voxeng> WARNING: Voxel at " << pos << " out of Chunk bounds.\n";
        return 0;
    }
    if (new_voxel >= m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel at " << pos << " of invalid Voxel Type " << new_voxel << ".\n";
        return 0;
    }
    size_t index = (pos.y * Chunk::DEPTH + pos.z) * Chunk::WIDTH + pos.x;
    VoxelID old = m_data[index];
    m_data[index] = new_voxel;
    return old;
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
                if (y < CHUNK_HEIGHT/2) { // /2
                    setVoxel({x,y,z}, 1);
                } else {
                    setVoxel({x,y,z}, 0);
                }
            }
        }
    }

    Vec3i pos{CHUNK_WIDTH-1, CHUNK_HEIGHT/2-1, CHUNK_DEPTH-1};
    setVoxel(pos, 0);
    pos = Vec3i{CHUNK_WIDTH-3, CHUNK_HEIGHT/2-1, CHUNK_DEPTH-3};
    setVoxel(pos, 0);

    setVoxel({CHUNK_WIDTH-5, CHUNK_HEIGHT/2, CHUNK_DEPTH-1}, 1);
    setVoxel({CHUNK_WIDTH-5, CHUNK_HEIGHT/2+1, CHUNK_DEPTH-1}, 1);
    setVoxel({CHUNK_WIDTH-5, CHUNK_HEIGHT/2+2, CHUNK_DEPTH-1}, 1);
}
