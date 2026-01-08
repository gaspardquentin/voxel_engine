#include "voxel_engine/world.h"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <queue>
#include <set>

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
        int render_dist_sq = m_render_distance * m_render_distance;
        _generateChunksBFS(render_dist_sq);
    }


    /**
     * Generates the world's chunks.
     * @param nb_to_generate the number of chunks to generate
     */
    void _generateChunksBFS(int nb_to_generate) {
        std::queue<Vec3f> bfs_queue;
        Vec3f chunk_pos = {0.0f, 0.0f, 0.0f};
        bfs_queue.push(chunk_pos);
        int nb_generated = 0;
        std::vector<Vec3f> bfs_visited;
        while (!bfs_queue.empty() && nb_generated < nb_to_generate) {
            chunk_pos = bfs_queue.front();
            bfs_queue.pop();
            m_chunks.push_back({m_voxel_types, chunk_pos});
            m_chunks.back().setRendererId(nb_generated);
            nb_generated++;
            const std::vector<Vec3f> neighboors = {
                chunk_pos + Vec3f{(float)Chunk::WIDTH, 0.0f, 0.0f}, // right
                chunk_pos + Vec3f{-((float)Chunk::WIDTH), 0.0f, 0.0f}, // left
                chunk_pos + Vec3f{0.0f, 0.0f, -((float)Chunk::DEPTH)}, // back
                chunk_pos + Vec3f{0.0f, 0.0f, (float)Chunk::DEPTH} // front
            };

            for (auto neighboor: neighboors) {
                if (std::find(bfs_visited.begin(), bfs_visited.end(), neighboor) == bfs_visited.end()) {
                    bfs_queue.push(neighboor);
                    bfs_visited.push_back(neighboor);
                }
            }
        }
    }

    size_t getChunkId(WorldCoord pos) const;
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
    size_t chunk_id = m_impl->getChunkId(pos);
    ChunkCoord chunk_pos = m_impl->m_chunks[chunk_id].getChunkPosFromWorld(pos);
    return m_impl->m_chunks[chunk_id].setVoxel(chunk_pos, new_voxel);
}


size_t World::Impl::getChunkId(WorldCoord pos) const {
    size_t index = (pos.y * Chunk::DEPTH + pos.z) * Chunk::WIDTH + pos.x;

    // estimation (reduces computation time by not checking every chunk)
    size_t chunk_id = (size_t) (index / Chunk::SIZE);
    if (chunk_id > m_chunks.size()) {
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        return 0;
    }

    if (m_chunks[chunk_id].worldPositionInChunk(pos)) {
        return chunk_id;
    }

    // look for chunks near the estimated one
    size_t start = (long) chunk_id - 5 < 0 ? chunk_id : chunk_id - 5;
    for (size_t i = start; i < start + 10; ++i) {
        if (i >= m_chunks.size()) {
            break;
        }
        if (m_chunks[i].worldPositionInChunk(pos)) {
            return i;
        }
    }

    // if still not found yet, do full iteration
    for (size_t i = 0; i < m_chunks.size(); i++) {
        if (m_chunks[i].worldPositionInChunk(pos)) {
            return i;
        }
    }


    std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
    return 0;

}

const VoxelType& World::getVoxel(WorldCoord pos) const {
    size_t chunk_id = m_impl->getChunkId(pos);
    ChunkCoord chunk_pos = m_impl->m_chunks[chunk_id].getChunkPosFromWorld(pos);
    return m_impl->m_chunks[chunk_id].getVoxel(chunk_pos);
}


/* TODO: remove this or implement this
std::optional<Vec3f> World::getVoxelTopLeftPos(Vec3f pos_inside) const {
    return {};
}
*/


std::vector<Chunk>& World::getChunks() const {
    return m_impl->m_chunks;
}

void World::update() {
    //TODO: implement this (if needed) (this might be great to use callbacks there)
    std::cerr << "Word::update(): TODO: implement this\n";
}

void World::setTextureForType(VoxelID vid, std::shared_ptr<Texture> texture) {
    if (vid >= m_impl->m_voxel_types.size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return;
    }
    m_impl->m_voxel_types[vid].setTexture(texture);
}

