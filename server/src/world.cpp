#include "voxel_engine/server/world.h"
#include "entt/entity/fwd.hpp"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/network/client_event.h"
#include "voxel_engine/network/i_server_connection.h"
#include "voxel_engine/save_format.h"
#include "voxel_engine/server/entity_components.h"
#include "voxel_engine/server/save_manager.h"
#include "voxel_engine/types.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/world_coords.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <entt/entt.hpp>

using namespace voxeng;

namespace voxeng::server {

class World::Impl {
public:
    network::IServerConnection& m_connection;
    std::vector<VoxelType> m_voxel_types;
    std::unordered_map<ChunkID, Chunk> m_chunks;
    std::vector<ChunkID> m_to_generate;
    uint8_t m_render_distance; //TODO: remove this from server world
    uint64_t m_seed;
    SaveManager* m_save_manager = nullptr;
    entt::registry m_registry;


    Impl(network::IServerConnection& connection, const std::vector<VoxelType>& voxel_types, uint8_t render_distance, uint64_t seed, bool generate_chunks):
        m_connection(connection),
        m_voxel_types(voxel_types),
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

    void _preGenerateChunks(int render_distance, ChunkID start_chunk) {
        m_to_generate.clear();
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
                    auto [it, _] = m_chunks.insert({chunk_id, {m_voxel_types, data.chunk_pos, std::move(data.voxels)}});
                    m_connection.pushEvent(network::ChunkDataEvent{chunk_id, it->second.getRawData()});
                    continue;
                }
            }

            // Generate new chunk
            auto chunk_pos = voxeng::getChunkWorldPos(chunk_id);
            auto [it, _] = m_chunks.insert({chunk_id, {m_voxel_types, chunk_pos}});
            m_connection.pushEvent(network::ChunkDataEvent{chunk_id, it->second.getRawData()});
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
                m_connection.pushEvent(network::ChunkUnloadEvent{it->first});
                it = m_chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

};

World::World(network::IServerConnection& connection, const std::vector<VoxelType>& voxel_types, uint64_t seed, bool generate_chunks): m_impl(std::make_unique<Impl>(connection, voxel_types, DEFAULT_RENDER_DISTANCE, seed, generate_chunks)) {}

World::~World() = default;
World::World(World&&) noexcept = default;
World& World::operator=(World&&) noexcept = default;

void World::setSaveManager(SaveManager* save_manager) {
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


VoxelID World::setVoxel(WorldCoord pos, VoxelID new_voxel) {
    ChunkID chunk_id = voxeng::getChunkId(pos);
    auto pair = m_impl->m_chunks.find(chunk_id);
    if (pair == m_impl->m_chunks.end()) {
        std::cerr << "<voxeng> WARNING: Voxel of pos" << pos << "is out of world bounds.\n";
        return 0;
    }
    Chunk& chunk = pair->second;
    ChunkCoord chunk_pos = chunk.getChunkPosFromWorld(pos);
    VoxelID old_voxel = chunk.setVoxel(chunk_pos, new_voxel);
    m_impl->m_connection.pushEvent(network::VoxelChangedEvent{chunk_id, chunk_pos, new_voxel});
    return old_voxel;
}

const VoxelType& World::getVoxel(WorldCoord pos) const {
    ChunkID chunk_id = voxeng::getChunkId(pos);
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

const Chunk *World::tryGetChunk(ChunkID cid) const {
    auto it = m_impl->m_chunks.find(cid);
    if (it == m_impl->m_chunks.end()) {
        return nullptr;
    }
    return &it->second;
}

void World::update() {
    int chunk_amount_to_gen = 2;
    m_impl->generateSomeChunks(chunk_amount_to_gen);
}


void World::setSeed(uint64_t seed) {
    m_impl->m_seed = seed;
}

uint64_t World::getSeed() const {
    return m_impl->m_seed;
}

entt::registry& World::getRegistry() {
    return m_impl->m_registry;
}
const entt::registry& World::getRegistry() const {
    return m_impl->m_registry;
}

entt::entity World::spawnEntity(std::string model_name, WorldCoord spawn_pos) {
    auto ent = m_impl->m_registry.create();

    m_impl->m_registry.emplace<Model>(ent, model_name);
    m_impl->m_registry.emplace<Position>(ent, spawn_pos);

    m_impl->m_connection.pushEvent(network::EntitySpawnEvent{
        static_cast<EntityID>(ent),
        model_name,
        spawn_pos
    });

    return ent;
}

void World::updateChunks(WorldCoord pos) {
    ChunkID player_chunk = voxeng::getChunkId(pos);
    m_impl->_preGenerateChunks(m_impl->m_render_distance, player_chunk);
    m_impl->_unloadDistantChunks(player_chunk);
}

}
