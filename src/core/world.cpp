#include "voxel_engine/world.h"
#include "voxel_engine/callbacks.h"
#include "chunk.h"
#include <cstdint>
#include <iostream>

class World::Impl {
public:
    ChunkGeneratorCallback m_chunk_generator;
    VoxelChangeCallback m_voxel_change_fun;
    std::vector<VoxelType> m_voxel_types;
    std::vector<Chunk> m_chunks;
    uint8_t m_render_distance;

    Impl(uint8_t render_distance):
        m_render_distance(render_distance),
        m_chunk_generator(DEFAULT_CHUNK_GENERATOR),
        m_voxel_change_fun(DEFAULT_VOXEL_CHANGE_FUNC),
        m_voxel_types(DEFAULT_VOXEL_TYPES) {
        generateChunks();
    }

    ~Impl() {}

    void generateChunks() {
        for (int i = 0; i < m_render_distance; i++) {
            m_chunks.push_back({m_voxel_types});
        }
    }
};

World::World(): m_impl(new Impl(DEFAULT_RENDER_DISTANCE)) {}
World::World(uint8_t render_distance): m_impl(new Impl(render_distance)) {}

World::~World() { delete m_impl; }


void World::setChunkGenerator(ChunkGeneratorCallback callback) {
    m_impl->m_chunk_generator = callback;
}

void World::setVoxelChange(VoxelChangeCallback callback) {
    m_impl->m_voxel_change_fun = callback;
}

void World::setVoxelTypes(std::vector<VoxelType> voxel_types) {
    m_impl->m_voxel_types = voxel_types;
}

const std::vector<VoxelType>& World::getVoxelTypes() const {
    return m_impl->m_voxel_types;
}

const VoxelType& World::getVoxelType(VoxelID vid) const {
    if (vid >= m_impl->m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return m_impl->m_voxel_types[0];
    }
    return m_impl->m_voxel_types[vid];
}


void World::setRenderDistance(uint8_t render_distance) {
    m_impl->m_render_distance = render_distance;
}


VoxelID World::setVoxel(WorldCoord pos, VoxelID new_voxel) {
    size_t index = (pos.y * Chunk::DEPTH + pos.z) * Chunk::WIDTH + pos.x;
    size_t chunk_id = index / Chunk::SIZE;
    if (chunk_id > m_impl->m_chunks.size()) {
        std::cout << chunk_id << std::endl;
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        return 0;
    }
    ChunkCoord chunk_pos{
        (int) (pos.x / (chunk_id * Chunk::WIDTH)),
        (int) (pos.y / (chunk_id * Chunk::HEIGHT)),
        (int) (pos.z / (chunk_id * Chunk::DEPTH))
    };
    return m_impl->m_chunks[chunk_id].setVoxel(chunk_pos, new_voxel);
}

const VoxelType& World::getVoxel(WorldCoord pos) const {
    size_t index = (pos.y * Chunk::DEPTH + pos.z) * Chunk::WIDTH + pos.x;
    std::cout << index << " " << index / CHUNK_SIZE << std::endl;
    size_t chunk_id = (size_t) (index / Chunk::WIDTH);
    if (chunk_id > m_impl->m_chunks.size()) {
        std::cout << chunk_id << std::endl;
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        return m_impl->m_voxel_types[0];
    }
    ChunkCoord chunk_pos{
        (int) (pos.x / (chunk_id * Chunk::WIDTH)),
        (int) (pos.y / (chunk_id * Chunk::HEIGHT)),
        (int) (pos.z / (chunk_id * Chunk::DEPTH))
    };
    return m_impl->m_chunks[chunk_id].getVoxel(chunk_pos);
}

void World::update() {

}

void World::setTextureForType(VoxelID vid, std::shared_ptr<Texture> texture) {
    if (vid >= m_impl->m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return;
    }
    m_impl->m_voxel_types[vid].setTexture(texture);
}

