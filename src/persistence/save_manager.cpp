#include "voxel_engine/save_manager.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/save_format.h"
#include "persistence/io/compressed_stream.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

class voxeng::SaveManager::Impl {
public:
    SaveID m_last_save = 0;
    std::string m_save_directory;

    Impl(std::string save_directory):
        m_save_directory(save_directory) {}

    std::string getSavePath(SaveID save) const {
        return m_save_directory + "save_" + std::to_string(save);
    }

    std::string getLastSavePath() const {
        return getSavePath(m_last_save);
    }
};

voxeng::SaveManager::SaveManager():
    m_impl(std::make_unique<Impl>(DEFAULT_SAVE_DIR)) {}

voxeng::SaveManager::SaveManager(std::string save_directory):
    m_impl(std::make_unique<Impl>(save_directory)) {}

void voxeng::SaveManager::setSaveDirectory(std::string save_directory) {
    //TODO: maybe verify it is a valid path
    m_impl->m_save_directory = save_directory;
}

std::string voxeng::SaveManager::getSaveDirectory() const {
    return m_impl->m_save_directory;
}

SaveID voxeng::SaveManager::getLastSave() {
    return m_impl->m_last_save;
}

std::string voxeng::SaveManager::saveWorld(const World& world) {
    std::string path = m_impl->getLastSavePath();
    voxeng::io::CompressedFileWriter writer(path);

    const WorldSaveData saveData = world.toData();

    writer.write(saveData.format_version);
    writer.write(saveData.format_sub_version);
    writer.write(saveData.world_seed);
    writer.write(saveData.chunks_nbr);

    for (const auto& chunk: saveData.chunks) {
        writer.write(chunk.chunk_pos.x);
        writer.write(chunk.chunk_pos.y);
        writer.write(chunk.chunk_pos.z);
        for (const auto& voxel: chunk.voxels) {
            writer.write(voxel);
        }
    }

    if (!writer.save()) {
        return "";
    }

    return path;
}

World voxeng::SaveManager::loadWorld(SaveID save) {
    std::string path = m_impl->getSavePath(save);
    voxeng::io::CompressedFileReader reader(path);

    if (!reader.load()) {
        return {0};
    }

    WorldSaveData saveData;
    reader.read(saveData.format_version);
    reader.read(saveData.format_sub_version);
    reader.read(saveData.world_seed);
    reader.read(saveData.chunks_nbr);

    for (size_t i = 0; i < saveData.chunks_nbr; i++) {
        ChunkSaveData chunk;
        reader.read(chunk.chunk_pos.x);
        reader.read(chunk.chunk_pos.y);
        reader.read(chunk.chunk_pos.z);
        for (auto& voxel: chunk.voxels) {
            reader.read(voxel);
        }
        saveData.chunks.push_back(chunk);
    }

    return World::fromData(saveData);
}

