#include "voxel_engine/chunk.h"
#include <iostream>

bool Chunk::positionInChunk(ChunkCoord pos) const {
    return  pos.x < Chunk::WIDTH &&
            pos.y < Chunk::HEIGHT &&
            pos.z < Chunk::DEPTH;
}

bool Chunk::worldPositionInChunk(Vec3f world_pos) const {
    return  world_pos.x >= m_world_pos.x && world_pos.x < m_world_pos.x + Chunk::WIDTH &&
            world_pos.y >= m_world_pos.y && world_pos.y < m_world_pos.y + Chunk::HEIGHT &&
            world_pos.z >= m_world_pos.z && world_pos.z < m_world_pos.z + Chunk::DEPTH;
}

ChunkCoord Chunk::getChunkPosFromWorld(Vec3f world_pos) const {
    if (!worldPositionInChunk(world_pos)) {
        std::cerr << "<voxeng> WARNING: Voxel at " << world_pos << " out of Chunk bounds.\n";
        return {0, 0, 0};
    }
    // TODO: make sure to get the exact block (issue converting float to int, might pick the neighboor)
    return ChunkCoord(
        static_cast<unsigned int>(world_pos.x - m_world_pos.x),
        static_cast<unsigned int>(world_pos.y - m_world_pos.y),
        static_cast<unsigned int>(world_pos.z - m_world_pos.z)
    );
}

const VoxelType& Chunk::getVoxel(ChunkCoord pos) const {
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


VoxelID Chunk::setVoxel(ChunkCoord pos, VoxelID new_voxel) {
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
    m_has_changed = true;
    return old;
}

const VoxelType& Chunk::getVoxelType(VoxelID vid) const {
    if (vid >= m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return m_voxel_types[0];
    }
    return m_voxel_types[vid];
}



Chunk::Chunk(const std::vector<VoxelType>& voxel_types, Vec3f position): 
    m_has_changed(true),
    m_renderer_id(0),
    m_voxel_types(voxel_types),
    m_world_pos(position)
{
    // TODO: replace with better chunk generation

    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
            for (unsigned int z = 0; z < CHUNK_DEPTH; z++) {
                if (y < CHUNK_HEIGHT/2 - 2) { // /2
                    setVoxel({x,y,z}, 1);
                }
                else if (y < CHUNK_HEIGHT/2) {
                    setVoxel({x,y,z}, 2);
                }
                else {
                    setVoxel({x,y,z}, 0);
                }
            }
        }
    }

    /*
    ChunkCoord pos{CHUNK_WIDTH-1, CHUNK_HEIGHT/2-1, CHUNK_DEPTH-1};
    setVoxel(pos, 0);
    pos = ChunkCoord{CHUNK_WIDTH-3, CHUNK_HEIGHT/2-1, CHUNK_DEPTH-3};
    setVoxel(pos, 0);

    setVoxel({CHUNK_WIDTH-5, CHUNK_HEIGHT/2, CHUNK_DEPTH-1}, 1);
    setVoxel({CHUNK_WIDTH-5, CHUNK_HEIGHT/2+1, CHUNK_DEPTH-1}, 1);
    setVoxel({CHUNK_WIDTH-5, CHUNK_HEIGHT/2+2, CHUNK_DEPTH-1}, 1);
    */
}
