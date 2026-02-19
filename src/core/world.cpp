#include "voxel_engine/world.h"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/camera.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/save_format.h"
#include "voxel_engine/save_manager.h"
#include "voxel_engine/voxel_types.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <unordered_map>

class World::Impl {
public:
    std::vector<VoxelType> m_voxel_types;
    std::unordered_map<ChunkID, Chunk> m_chunks;
    std::vector<ChunkID> m_to_generate;
    uint8_t m_render_distance;
    uint64_t m_seed;
    voxeng::SaveManager* m_save_manager = nullptr;

    Impl(uint8_t render_distance, uint64_t seed, bool generate_chunks):
        m_voxel_types(DEFAULT_VOXEL_TYPES),
        m_render_distance(render_distance),
        m_seed(seed) {
        if (generate_chunks) {
            generateChunksInit();
        }
    }

    ~Impl() {}

    void generateChunksInit() {
        _preGenerateChunks(m_render_distance, {0, 0});
        generateSomeChunks(m_render_distance);
    }

    Vec3f _getChunkPosFromId(ChunkID chunk_id) {
        return {
            (float) chunk_id.x * CHUNK_WIDTH,
            0.0f,
            (float) chunk_id.y * CHUNK_DEPTH
        };
    }

    void _preGenerateChunks(int render_distance, ChunkID start_chunk) {
        for (int y = -render_distance; y <= render_distance; y++) {
            for (int x = -render_distance; x <= render_distance; x++) {
                ChunkID cid = {start_chunk.x + x, start_chunk.y + y};
                if (m_chunks.find(cid) == m_chunks.end()) {
                    m_to_generate.push_back(cid);
                }
            }
        }

        std::sort(m_to_generate.begin(), m_to_generate.end(),
            [&start_chunk](const ChunkID& a, const ChunkID& b) {
                return ChunkID::chebyshev(start_chunk, a) > ChunkID::chebyshev(start_chunk, b);
            });
    }

    void generateSomeChunks(int amount_max) {
        if (m_to_generate.size() == 0) {
            return;
        }

        int count = std::min(amount_max, static_cast<int>(m_to_generate.size()));
        for (int i = 0; i < count; i++) {
            ChunkID chunk_id = m_to_generate.back();
            m_to_generate.pop_back();

            // Try to load from disk first
            if (m_save_manager && m_save_manager->chunkExistsOnDisk(chunk_id)) {
                ChunkSaveData data;
                if (m_save_manager->loadChunk(chunk_id, data)) {
                    m_chunks.insert({chunk_id, {m_voxel_types, data.chunk_pos, std::move(data.voxels)}});
                    continue;
                }
            }

            // Generate new chunk
            auto chunk_pos = _getChunkPosFromId(chunk_id);
            m_chunks.insert({chunk_id, {m_voxel_types, chunk_pos}});
        }
    }

    void _unloadDistantChunks(ChunkID player_chunk) {
        int unload_distance = m_render_distance + 2;
        for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
            if (ChunkID::chebyshev(player_chunk, it->first) > unload_distance) {
                // Save dirty chunk before unloading
                if (m_save_manager && m_save_manager->isWorldOpen() && it->second.isPersistenceDirty()) {
                    ChunkSaveData data;
                    data.chunk_pos = it->second.getWorldPos();
                    data.voxels = it->second.getRawData();
                    m_save_manager->saveChunk(it->first, data);
                }
                it = m_chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

    static ChunkID getChunkId(WorldCoord pos);
};

World::World(uint64_t seed, bool generate_chunks): m_impl(std::make_unique<Impl>(DEFAULT_RENDER_DISTANCE, seed, generate_chunks)) {}
World::World(uint8_t render_distance, uint64_t seed, bool generate_chunks): m_impl(std::make_unique<Impl>(render_distance, seed, generate_chunks)) {}

World::~World() = default;
World::World(World&&) noexcept = default;
World& World::operator=(World&&) noexcept = default;

void World::setSaveManager(voxeng::SaveManager* save_manager) {
    m_impl->m_save_manager = save_manager;
}

void World::flushAllDirtyChunks() {
    if (!m_impl->m_save_manager || !m_impl->m_save_manager->isWorldOpen()) return;

    for (auto& [id, chunk] : m_impl->m_chunks) {
        if (chunk.isPersistenceDirty()) {
            ChunkSaveData data;
            data.chunk_pos = chunk.getWorldPos();
            data.voxels = chunk.getRawData();
            m_impl->m_save_manager->saveChunk(id, data);
            chunk.clearPersistenceDirty();
        }
    }
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

uint8_t World::getRenderDistance() const {
    return m_impl->m_render_distance;
}

VoxelID World::setVoxel(WorldCoord pos, VoxelID new_voxel) {
    ChunkID chunk_id = Impl::getChunkId(pos);
    auto pair = m_impl->m_chunks.find(chunk_id);
    if (pair == m_impl->m_chunks.end()) {
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
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
        return m_impl->m_voxel_types[0];
    }
    Chunk& chunk = pair->second;
    ChunkCoord chunk_pos = chunk.getChunkPosFromWorld(pos);
    return chunk.getVoxel(chunk_pos);
}

const std::unordered_map<ChunkID, Chunk>& World::getChunks() const {
    return m_impl->m_chunks;
}

void World::update() {
    int chunk_amount_to_gen = 2;
    m_impl->generateSomeChunks(chunk_amount_to_gen);
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

void World::updateChunks(WorldCoord pos) {
    ChunkID player_chunk = Impl::getChunkId(pos);
    m_impl->_preGenerateChunks(m_impl->m_render_distance, player_chunk);
    m_impl->_unloadDistantChunks(player_chunk);
}
