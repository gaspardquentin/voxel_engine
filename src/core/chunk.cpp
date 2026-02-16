#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"
#include <array>
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

const std::array<VoxelID, CHUNK_SIZE>& Chunk::getRawData() const {
    return m_data;
}

/* TODO; maybe remove this
void Chunk::setRawData(std::array<VoxelID, CHUNK_SIZE>&& new_data) {
    m_data = std::move(new_data);
}
*/

Chunk::Chunk(const std::vector<VoxelType>& voxel_types, Vec3f position): 
    m_has_changed(true),
    m_renderer_id(0),
    m_world_pos(position),
    m_voxel_types(voxel_types)
{
    // TODO: replace with better chunk generation
    // TODO: maybe separate the generation logic from this constructor

    // Basic terrain generation
    for (unsigned int x = 0; x < CHUNK_WIDTH; x++) {
        for (unsigned int z = 0; z < CHUNK_DEPTH; z++) {
            int ground_height = CHUNK_HEIGHT / 2;
            
            for (unsigned int y = 0; y < CHUNK_HEIGHT; y++) {
                if (y < ground_height - 2) {
                    setVoxel({x,y,z}, 3); // Stone
                } else if (y < ground_height) {
                    setVoxel({x,y,z}, 2); // Dirt
                } else if (y == ground_height) {
                    setVoxel({x,y,z}, 5); // Grass
                } else {
                    setVoxel({x,y,z}, 0); // Air
                }
            }
        }
    }

    // Add a tree in the center
    int tree_x = CHUNK_WIDTH / 2;
    int tree_z = CHUNK_DEPTH / 2;
    int ground = CHUNK_HEIGHT / 2;
    
    // Trunk
    for (int i = 1; i <= 4; i++) {
        setVoxel({(unsigned int)tree_x, (unsigned int)(ground + i), (unsigned int)tree_z}, 6); // Wood
    }
    
    // Leaves
    for (int ly = ground + 3; ly <= ground + 5; ly++) {
        for (int lx = tree_x - 2; lx <= tree_x + 2; lx++) {
            for (int lz = tree_z - 2; lz <= tree_z + 2; lz++) {
                 // Skip corners to make it rounder
                 if (std::abs(lx - tree_x) == 2 && std::abs(lz - tree_z) == 2) continue;

                 if (lx == tree_x && lz == tree_z && ly < ground + 5) continue; // Don't overwrite trunk (except top)
                 
                 ChunkCoord pos = {(unsigned int)lx, (unsigned int)ly, (unsigned int)lz};
                 if (positionInChunk(pos)) {
                     setVoxel(pos, 7); // Leaves
                 }
            }
        }
    }

    // Water pool
    for (unsigned int x = 1; x < 4; x++) {
        for (unsigned int z = 1; z < 4; z++) {
            setVoxel({x, (unsigned int)ground, z}, 8); // Water
            setVoxel({x, (unsigned int)ground - 1, z}, 4); // Sand
        }
    }
    
    // Some glass/brick structure
    setVoxel({CHUNK_WIDTH - 2, (unsigned int)ground + 1, CHUNK_DEPTH - 2}, 9); // Brick
    setVoxel({CHUNK_WIDTH - 2, (unsigned int)ground + 2, CHUNK_DEPTH - 2}, 10); // Glass
}


Chunk::Chunk(const std::vector<VoxelType>& voxel_types, Vec3f position, std::array<VoxelID, CHUNK_SIZE>&& raw_data):
    m_has_changed(true),
    m_renderer_id(0),
    m_world_pos(position),
    m_voxel_types(voxel_types),
    m_data(std::move(raw_data)) {}
