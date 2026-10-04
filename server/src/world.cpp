#include "voxel_engine/server/world.h"
#include "entt/entity/fwd.hpp"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/network/client_event.h"
#include "voxel_engine/network/i_server_connection.h"
#include "voxel_engine/save_format.h"
#include "voxel_engine/server/chunk_generators.h"
#include "voxel_engine/server/entity_components.h"
#include "voxel_engine/server/save_manager.h"
#include "voxel_engine/types.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/world_coords.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <entt/entt.hpp>

using namespace voxeng;

namespace voxeng::server {

// Max chunks loaded from disk per tick (zlib decompression runs on the server thread)
static constexpr size_t MAX_DISK_LOADS_PER_TICK = 8;
// Chunks farther than render distance + this margin get unloaded
static constexpr int UNLOAD_MARGIN = 2;

// Shared with worker tasks so they can outlive the World that submitted them
struct GenOutbox {
    std::mutex mutex;
    std::vector<std::pair<ChunkID, RawChunk>> chunks;
};

class World::Impl {
public:
    network::IServerConnection& m_connection;
    std::vector<VoxelType> m_voxel_types;
    std::unordered_map<ChunkID, Chunk> m_chunks;
    std::vector<ChunkID> m_to_generate;
    std::unordered_map<UserID, entt::entity> m_player_entities;
    uint64_t m_seed;
    SaveManager* m_save_manager = nullptr;
    std::shared_ptr<const IChunkGenerator> m_chunk_generator;
    entt::registry m_registry;
    ThreadPool& m_thread_pool;
    std::shared_ptr<GenOutbox> m_gen_outbox = std::make_shared<GenOutbox>();
    // chunks submitted to the thread pool but not yet collected
    std::unordered_set<ChunkID> m_pending;


    Impl(network::IServerConnection& connection, const std::vector<VoxelType>& voxel_types, uint64_t seed, bool generate_chunks, std::unique_ptr<IChunkGenerator> chunk_generator, ThreadPool& thread_pool):
        m_connection(connection),
        m_voxel_types(voxel_types),
        m_seed(seed),
        m_chunk_generator(std::move(chunk_generator)),
        m_thread_pool(thread_pool)
        {
        if (generate_chunks) {
            generateChunksInit();
        }
    }

    ~Impl() {}

    void generateChunksInit() {
        _preGenerateChunks(DEFAULT_RENDER_DISTANCE, {0, 0});
        generateSomeChunks(DEFAULT_RENDER_DISTANCE);
    }

    void _preGenerateChunks(int render_distance, ChunkID start_chunk) {
        m_to_generate.clear();
        for (int y = -render_distance; y <= render_distance; y++) {
            for (int x = -render_distance; x <= render_distance; x++) {
                ChunkID cid = {start_chunk.x + x, start_chunk.y + y};
                if (m_chunks.find(cid) == m_chunks.end() && m_pending.find(cid) == m_pending.end()) {
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
            auto raw_data = m_chunk_generator->generate(chunk_id);
            auto [it, _] = m_chunks.insert({chunk_id, {m_voxel_types, chunk_pos, std::move(raw_data)}});
            m_connection.pushEvent(network::ChunkDataEvent{chunk_id, it->second.getRawData()});
        }
    }

    // Whether a chunk is still within unload distance of at least one player
    bool isChunkWanted(ChunkID id) {
        for (auto [_, player, tracking]: m_registry.view<PlayerData, ChunkTracking>().each()) {
            if (ChunkID::chebyshev(tracking.last_chunk, id) <= player.render_distance + UNLOAD_MARGIN) {
                return true;
            }
        }
        return false;
    }

    void _unloadDistantChunks(int render_distance, ChunkID player_chunk) {
        int unload_distance = render_distance + UNLOAD_MARGIN;
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

World::World(network::IServerConnection& connection, const std::vector<VoxelType>& voxel_types, uint64_t seed, std::unique_ptr<IChunkGenerator> generator, ThreadPool& thread_pool, bool generate_chunks): m_impl(std::make_unique<Impl>(connection, voxel_types, seed, generate_chunks, std::move(generator), thread_pool)) {}

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
    auto& impl = *m_impl;

    // submit generation tasks to thread pool, nearest first (m_to_generate is sorted farthest first)
    size_t disk_loads = 0;
    while (!impl.m_to_generate.empty()) {
        ChunkID id = impl.m_to_generate.back();

        // Try to load from disk first (on main thread, SaveManager isn't thread-safe)
        // TODO: maybe make SaveManager thread-safe
        if (impl.m_save_manager && impl.m_save_manager->chunkExistsOnDisk(id)) {
            if (disk_loads >= MAX_DISK_LOADS_PER_TICK) {
                break; // keep the remainder for the next tick
            }
            disk_loads++;
            ChunkSaveData data;
            if (impl.m_save_manager->loadChunk(id, data)) {
                impl.m_to_generate.pop_back();
                auto [it, _] = impl.m_chunks.insert({id, {impl.m_voxel_types, data.chunk_pos, std::move(data.voxels)}});
                impl.m_connection.pushEvent(network::ChunkDataEvent{id, it->second.getRawData()});
                continue;
            }
        }
        impl.m_to_generate.pop_back();

        // Not on disk, generate on worker thread
        impl.m_pending.insert(id);
        impl.m_thread_pool.submit([generator = impl.m_chunk_generator, outbox = impl.m_gen_outbox, id]() {
            RawChunk rc = generator->generate(id);
            std::lock_guard<std::mutex> lock(outbox->mutex);
            outbox->chunks.push_back({id, std::move(rc)});
        });
    }

    std::vector<std::pair<ChunkID, RawChunk>> generated;
    {
        std::lock_guard<std::mutex> lock(impl.m_gen_outbox->mutex);
        generated.swap(impl.m_gen_outbox->chunks);
    }
    for (auto& [id, data]: generated) {
        impl.m_pending.erase(id);
        // the player may have moved away while this chunk was being generated
        if (!impl.isChunkWanted(id)) {
            continue;
        }
        auto chunk_pos = getChunkWorldPos(id);
        auto [it, inserted] = impl.m_chunks.insert({id, {impl.m_voxel_types, chunk_pos, std::move(data)}});
        if (inserted) {
            impl.m_connection.pushEvent(network::ChunkDataEvent{id, it->second.getRawData()});
        }
    }
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

void World::playerJoin(UserProfile player, WorldCoord spawn_pos, uint8_t render_distance) {
    auto ent = m_impl->m_registry.create();

    m_impl->m_registry.emplace<PlayerData>(ent, PlayerData{player, render_distance});
    m_impl->m_registry.emplace<Position>(ent, spawn_pos);
    m_impl->m_registry.emplace<ChunkTracking>(ent);

    m_impl->m_player_entities[player.id] = ent;
}


const std::unordered_map<UserID, entt::entity>& World::getPlayerEntities() const {
    return m_impl->m_player_entities;
}

void World::updateChunks() {
    for (auto [_, player, pos, tracking]: m_impl->m_registry.view<PlayerData, Position, ChunkTracking>().each()) {
        ChunkID current_chunk = voxeng::getChunkId(pos.pos);
        if (current_chunk == tracking.last_chunk) continue;
        tracking.last_chunk = current_chunk;
        m_impl->_preGenerateChunks(player.render_distance, current_chunk);
        m_impl->_unloadDistantChunks(player.render_distance, current_chunk);
    }
}

}
