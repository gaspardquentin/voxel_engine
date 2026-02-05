#include "voxel_engine/world.h"
#include "rendering/camera.h"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/save_format.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>

class World::Impl {
public:
    //TODO: use this when callback system is implemented
    //ChunkGeneratorCallback m_chunk_generator;
    //VoxelChangeCallback m_voxel_change_fun;
    std::vector<VoxelType> m_voxel_types;
    //std::vector<Chunk> m_chunks; TODO: remove this
    std::unordered_map<ChunkID, Chunk> m_chunks;
    uint8_t m_render_distance;
    uint64_t m_seed;

    Impl(uint8_t render_distance, uint64_t seed):
        //m_chunk_generator(DEFAULT_CHUNK_GENERATOR),
        //m_voxel_change_fun(DEFAULT_VOXEL_CHANGE_FUNC),
        m_voxel_types(DEFAULT_VOXEL_TYPES),
        m_render_distance(render_distance),
        m_seed(seed) {
        generateChunks();
    }

    ~Impl() {}

    void generateChunks() {
        int render_dist_sq = m_render_distance * m_render_distance;
        _generateChunksBFS(render_dist_sq);
    }



    // Retrieve the actual position of the chunk in the world map by it's relative position 
    // to the center chunk. TODO: make this doc more clear
    Vec3f _getChunkPosFromId(ChunkID chunk_id) {
        return {
            (float) chunk_id.x * CHUNK_WIDTH,
            0.0f,
            (float) chunk_id.y * CHUNK_DEPTH
        };
    }



    /**
     * Generates the world's chunks.
     * @param nb_to_generate the number of chunks to generate
     */
    void _generateChunksBFS(int nb_to_generate) {
        std::queue<ChunkID> bfs_queue;

        // Note: see this as a vector of position relative to the center chunk.
        // since chunks occupies all the y-axis, this vector should be interpreted
        // as {x, z} components rather than the usual {x, y} TODO: maybe make this note more clear
        ChunkID chunk_id = {0, 0};

        bfs_queue.push(chunk_id);
        std::unordered_set<ChunkID> bfs_visited;
        bfs_visited.insert(chunk_id);

        int nb_generated = 0;
        while (!bfs_queue.empty() && nb_generated < nb_to_generate) {
            chunk_id = bfs_queue.front();
            bfs_queue.pop();
            auto chunk_pos = _getChunkPosFromId(chunk_id);
            m_chunks.insert({chunk_id, {m_voxel_types, chunk_pos}});
            //m_chunks.push_back({m_voxel_types, chunk_pos});
            m_chunks.at(chunk_id).setRendererId(nb_generated);
            //m_chunks[chunk_id].setRendererId(nb_generated);
            //m_chunks.back().setRendererId(nb_generated);
            nb_generated++;

            const std::vector<Vec2i> neighboors = {
                chunk_id + Vec2i{1, 0}, // right
                chunk_id + Vec2i{-1, 0}, // left
                chunk_id + Vec2i{0, -1}, // back
                chunk_id + Vec2i{0, 1} // front
            };

            /*
            const std::vector<Vec3f> neighboors = {
                chunk_pos + Vec3f{(float)Chunk::WIDTH, 0.0f, 0.0f}, // right
                chunk_pos + Vec3f{-((float)Chunk::WIDTH), 0.0f, 0.0f}, // left
                chunk_pos + Vec3f{0.0f, 0.0f, -((float)Chunk::DEPTH)}, // back
                chunk_pos + Vec3f{0.0f, 0.0f, (float)Chunk::DEPTH} // front
            };
            */

            for (auto neighboor: neighboors) {
                if (bfs_visited.find(neighboor) == bfs_visited.end()) {
                //if (std::find(bfs_visited.begin(), bfs_visited.end(), neighboor) == bfs_visited.end()) {
                    bfs_queue.push(neighboor);
                    //bfs_visited.push_back(neighboor);
                    bfs_visited.insert(neighboor);
                }
            }
        }
    }

    static ChunkID getChunkId(WorldCoord pos);
};

World::World(uint64_t seed): m_impl(new Impl(DEFAULT_RENDER_DISTANCE, seed)) {}
World::World(uint8_t render_distance, uint64_t seed): m_impl(new Impl(render_distance, seed)) {}

World::~World() { delete m_impl; }


/* TODO: use those once callback system is implemented
void World::setChunkGenerator(ChunkGeneratorCallback callback) {
    m_impl->m_chunk_generator = callback;
}

void World::setVoxelChange(VoxelChangeCallback callback) {
    m_impl->m_voxel_change_fun = callback;
}
*/

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

uint8_t World::getRenderDistance() const {
    return m_impl->m_render_distance;
}

VoxelID World::setVoxel(WorldCoord pos, VoxelID new_voxel) {
    ChunkID chunk_id = Impl::getChunkId(pos);
    auto pair = m_impl->m_chunks.find(chunk_id);
    if (pair == m_impl->m_chunks.end()) {
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        //TODO: maybe return something else
        return 0;
    }
    Chunk& chunk = pair->second;
    ChunkCoord chunk_pos = chunk.getChunkPosFromWorld(pos);
    return chunk.setVoxel(chunk_pos, new_voxel);
}

ChunkID World::Impl::getChunkId(WorldCoord pos) {
    return {
        static_cast<int32_t>(std::floor(pos.x / static_cast<float>(Chunk::WIDTH))),
        static_cast<int32_t>(std::floor(pos.z / static_cast<float>(Chunk::DEPTH)))
    };
}

const VoxelType& World::getVoxel(WorldCoord pos) const {
    ChunkID chunk_id = Impl::getChunkId(pos);
    auto pair = m_impl->m_chunks.find(chunk_id);
    if (pair == m_impl->m_chunks.end()) {
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        //TODO: maybe return something else
        return m_impl->m_voxel_types[0];
    }
    Chunk& chunk = pair->second;
    ChunkCoord chunk_pos = chunk.getChunkPosFromWorld(pos);
    return chunk.getVoxel(chunk_pos);
}


/* TODO: remove this or implement this
std::optional<Vec3f> World::getVoxelTopLeftPos(Vec3f pos_inside) const {
    return {};
}
*/


const std::unordered_map<ChunkID, Chunk>& World::getChunks() const {
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



void World::setSeed(uint64_t seed) {
    m_impl->m_seed = seed;
}

uint64_t World::getSeed() const {
    return m_impl->m_seed;
}

World World::fromData(const WorldSaveData& data) {

    return World(data.world_seed);
}

WorldSaveData World::toData() const {
    WorldSaveData save;
    save.world_seed = m_impl->m_seed;
    save.chunks_nbr = m_impl->m_chunks.size();
    for (const auto& [_, chunk]: m_impl->m_chunks) {
        ChunkSaveData chunk_data;
        chunk_data.chunk_pos = chunk.getWorldPos();
        const auto& voxels = chunk.getRawData();
        for (size_t i = 0; i < chunk_data.voxels.size(); i++) {
            chunk_data.voxels[i].type = voxels[i];
        }
        save.chunks.push_back(chunk_data);
    }
    return save;
}

