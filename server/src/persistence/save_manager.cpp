#include "voxel_engine/server/save_manager.h"
#include "voxel_engine/callbacks.h"
#include "voxel_engine/save_format.h"
#include "persistence/io/compressed_stream.h"
#include "voxel_engine/user.h"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

using namespace voxeng;

class voxeng::server::SaveManager::Impl {
public:
    std::string m_world_path;
    WorldMetadata m_metadata;
    bool m_is_open = false;

    std::string chunkFilePath(ChunkID id) const {
        return m_world_path + "chunks/" + std::to_string(id.x) + "_" + std::to_string(id.y) + ".dat";
    }

    std::string metaFilePath() const {
        return m_world_path + "world.meta";
    }

    bool writeMetadata() {
        voxeng::server::io::CompressedFileWriter writer(metaFilePath());
        writer.write(m_metadata.format_version);
        writer.write(m_metadata.format_sub_version);
        writer.write(m_metadata.world_seed);

        // Voxel types (TODO: maybe separate this into a submethod)
        uint16_t num_types = static_cast<uint16_t>(m_metadata.voxel_types.size());
        writer.write(num_types);
        for (const auto& vt : m_metadata.voxel_types) {
            writer.write(vt.getId());
            writer.writeString(vt.getName());
            writer.writeString(vt.getTexture());
            uint8_t solid = vt.isSolid() ? 1 : 0;
            uint8_t transparent = vt.isTransparent() ? 1 : 0;
            writer.write(solid);
            writer.write(transparent);
        }

        // Players data (TODO: maybe separate this into a submethod)
        uint32_t num_players = static_cast<uint32_t>(m_metadata.known_players.size());
        writer.write(num_players);
        for (const auto& pdata: m_metadata.known_players) {
            writer.write(pdata.uid);
            writer.write(pdata.position.x);
            writer.write(pdata.position.y);
            writer.write(pdata.position.z);
        }

        return writer.save();
    }

    bool readMetadata() {
        voxeng::server::io::CompressedFileReader reader(metaFilePath());
        if (!reader.load()) return false;
        reader.read(m_metadata.format_version);
        reader.read(m_metadata.format_sub_version);
        reader.read(m_metadata.world_seed);

        if (m_metadata.format_sub_version >= 1) {
            // Voxel types (TODO: again, maybe separate into a submethod)
            uint16_t num_types = 0;
            reader.read(num_types);
            m_metadata.voxel_types.clear();
            for (uint16_t i = 0; i < num_types; ++i) {
                VoxelID id = 0;
                std::string name = "";
                std::string texture = "";
                uint8_t solid = 0;
                uint8_t transparent = 0;
                reader.read(id);
                reader.readString(name);
                reader.readString(texture);
                reader.read(solid);
                reader.read(transparent);
                m_metadata.voxel_types.push_back({id, {std::move(name), std::move(texture), solid != 0, transparent != 0}});
            }

            // Players data (TODO: again, maybe separate into a submethod)
            uint32_t num_players = 0;
            reader.read(num_players);
            m_metadata.known_players.clear();
            for (uint32_t i = 0; i < num_players; i++) {
                UserID uid = 0;
                WorldCoord pos = {0.0f, 0.0f, 0.0f};
                reader.read(uid);
                reader.read(pos.x);
                reader.read(pos.y);
                reader.read(pos.z);
                m_metadata.known_players.push_back({uid, pos});
            }
        }
        return true;
    }
};

voxeng::server::SaveManager::SaveManager():
    m_impl(std::make_unique<Impl>()) {}

voxeng::server::SaveManager::~SaveManager() = default;

bool voxeng::server::SaveManager::createWorld(const std::string& name, uint64_t seed,
                                               const std::vector<VoxelType>& voxel_types) {
    std::string path = std::string(DEFAULT_WORLDS_DIR) + name + "/";

    try {
        std::filesystem::create_directories(path + "chunks/");
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "<voxeng> SaveManager: failed to create world directory: " << e.what() << "\n";
        return false;
    }

    m_impl->m_world_path = path;
    m_impl->m_metadata = {};
    m_impl->m_metadata.world_seed = seed;
    m_impl->m_metadata.voxel_types = voxel_types;
    m_impl->m_is_open = true;

    if (!m_impl->writeMetadata()) {
        std::cerr << "<voxeng> SaveManager: failed to write world.meta\n";
        m_impl->m_is_open = false;
        return false;
    }

    return true;
}

bool voxeng::server::SaveManager::openWorld(const std::string& world_path) {
    std::string path = world_path;
    if (!path.empty() && path.back() != '/') path += '/';

    if (!std::filesystem::exists(path + "world.meta")) {
        std::cerr << "<voxeng> SaveManager: world.meta not found in " << path << "\n";
        return false;
    }

    m_impl->m_world_path = path;
    if (!m_impl->readMetadata()) {
        std::cerr << "<voxeng> SaveManager: failed to read world.meta\n";
        return false;
    }

    m_impl->m_is_open = true;
    return true;
}

void voxeng::server::SaveManager::closeWorld() {
    m_impl->m_is_open = false;
    m_impl->m_world_path.clear();
    m_impl->m_metadata = {};
}

bool voxeng::server::SaveManager::isWorldOpen() const {
    return m_impl->m_is_open;
}

bool voxeng::server::SaveManager::saveChunk(ChunkID id, const ChunkSaveData& data) {
    if (!m_impl->m_is_open) return false;

    voxeng::server::io::CompressedFileWriter writer(m_impl->chunkFilePath(id));
    writer.write(data.chunk_pos.x);
    writer.write(data.chunk_pos.y);
    writer.write(data.chunk_pos.z);
    for (const auto& voxel : data.voxels) {
        writer.write(voxel);
    }
    return writer.save();
}

bool voxeng::server::SaveManager::loadChunk(ChunkID id, ChunkSaveData& data) {
    if (!m_impl->m_is_open) return false;

    std::string path = m_impl->chunkFilePath(id);
    voxeng::server::io::CompressedFileReader reader(path);
    if (!reader.load()) return false;

    reader.read(data.chunk_pos.x);
    reader.read(data.chunk_pos.y);
    reader.read(data.chunk_pos.z);
    for (auto& voxel : data.voxels) {
        reader.read(voxel);
    }
    return true;
}

bool voxeng::server::SaveManager::chunkExistsOnDisk(ChunkID id) const {
    if (!m_impl->m_is_open) return false;
    return std::filesystem::exists(m_impl->chunkFilePath(id));
}

const WorldMetadata& voxeng::server::SaveManager::getMetadata() const {
    return m_impl->m_metadata;
}

bool voxeng::server::SaveManager::updateMetadata(const WorldMetadata& meta) {
    if (!m_impl->m_is_open) return false;
    m_impl->m_metadata = meta;
    return m_impl->writeMetadata();
}

std::vector<std::string> voxeng::server::SaveManager::listWorlds(const std::string& base_dir) {
    std::vector<std::string> worlds;
    if (!std::filesystem::exists(base_dir)) return worlds;

    for (const auto& entry : std::filesystem::directory_iterator(base_dir)) {
        if (entry.is_directory()) {
            std::string meta_path = entry.path().string() + "/world.meta";
            if (std::filesystem::exists(meta_path)) {
                worlds.push_back(entry.path().string());
            }
        }
    }
    return worlds;
}
