#include "voxel_engine/world.h"
#include "voxel_engine/callbacks.h"

class World::Impl {
public:
    ChunkGeneratorCallback m_chunk_generator;
    VoxelChangeCallback m_voxel_change_fun;
    std::vector<VoxelType> m_voxel_types;

    Impl():
        m_chunk_generator(DEFAULT_CHUNK_GENERATOR),
        m_voxel_change_fun(DEFAULT_VOXEL_CHANGE_FUNC),
        m_voxel_types(DEFAULT_VOXEL_TYPES) {}

    ~Impl();
};


void World::setChunkGenerator(ChunkGeneratorCallback callback) {
    m_impl->m_chunk_generator = callback;
}

void World::setVoxelChange(VoxelChangeCallback callback) {
    m_impl->m_voxel_change_fun = callback;
}

void World::setVoxelTypes(std::vector<VoxelType> voxel_types) {
    m_impl->m_voxel_types = voxel_types;
}

