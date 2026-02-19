#include "voxel_engine/save_manager.h"
#include "voxel_engine/save_format.h"
#include "persistence/io/compressed_stream.h"
#include <filesystem>
#include <iostream>
#include <string>

class voxeng::SaveManager::Impl {
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
        voxeng::io::CompressedFileWriter writer(metaFilePath());
        writer.write(m_metadata.format_version);
        writer.write(m_metadata.format_sub_version);
        writer.write(m_metadata.world_seed);
        return writer.save();
    }

    bool readMetadata() {
        voxeng::io::CompressedFileReader reader(metaFilePath());
        if (!reader.load()) return false;
        reader.read(m_metadata.format_version);
        reader.read(m_metadata.format_sub_version);
        reader.read(m_metadata.world_seed);
        return true;
    }
};

voxeng::SaveManager::SaveManager():
    m_impl(std::make_unique<Impl>()) {}

voxeng::SaveManager::~SaveManager() = default;

bool voxeng::SaveManager::createWorld(const std::string& name, uint64_t seed) {
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
    m_impl->m_is_open = true;

    if (!m_impl->writeMetadata()) {
        std::cerr << "<voxeng> SaveManager: failed to write world.meta\n";
        m_impl->m_is_open = false;
        return false;
    }

    return true;
}

bool voxeng::SaveManager::openWorld(const std::string& world_path) {
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

void voxeng::SaveManager::closeWorld() {
    m_impl->m_is_open = false;
    m_impl->m_world_path.clear();
    m_impl->m_metadata = {};
}

bool voxeng::SaveManager::isWorldOpen() const {
    return m_impl->m_is_open;
}

bool voxeng::SaveManager::saveChunk(ChunkID id, const ChunkSaveData& data) {
    if (!m_impl->m_is_open) return false;

    voxeng::io::CompressedFileWriter writer(m_impl->chunkFilePath(id));
    writer.write(data.chunk_pos.x);
    writer.write(data.chunk_pos.y);
    writer.write(data.chunk_pos.z);
    for (const auto& voxel : data.voxels) {
        writer.write(voxel);
    }
    return writer.save();
}

bool voxeng::SaveManager::loadChunk(ChunkID id, ChunkSaveData& data) {
    if (!m_impl->m_is_open) return false;

    std::string path = m_impl->chunkFilePath(id);
    voxeng::io::CompressedFileReader reader(path);
    if (!reader.load()) return false;

    reader.read(data.chunk_pos.x);
    reader.read(data.chunk_pos.y);
    reader.read(data.chunk_pos.z);
    for (auto& voxel : data.voxels) {
        reader.read(voxel);
    }
    return true;
}

bool voxeng::SaveManager::chunkExistsOnDisk(ChunkID id) const {
    if (!m_impl->m_is_open) return false;
    return std::filesystem::exists(m_impl->chunkFilePath(id));
}

const WorldMetadata& voxeng::SaveManager::getMetadata() const {
    return m_impl->m_metadata;
}

bool voxeng::SaveManager::updateMetadata(const WorldMetadata& meta) {
    if (!m_impl->m_is_open) return false;
    m_impl->m_metadata = meta;
    return m_impl->writeMetadata();
}

std::vector<std::string> voxeng::SaveManager::listWorlds(const std::string& base_dir) {
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
