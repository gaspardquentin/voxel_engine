#pragma once

#include "voxel_engine/save_format.h"
#include "voxel_engine/math_utils.h"
#include <memory>
#include <string>
#include <vector>

#define DEFAULT_WORLDS_DIR "worlds/"

namespace voxeng {

class SaveManager {
    class Impl;
    std::unique_ptr<Impl> m_impl;
public:
    SaveManager();
    ~SaveManager();

    bool createWorld(const std::string& name, uint64_t seed);
    bool openWorld(const std::string& world_path);
    void closeWorld();
    bool isWorldOpen() const;

    bool saveChunk(ChunkID id, const ChunkSaveData& data);
    bool loadChunk(ChunkID id, ChunkSaveData& data);
    bool chunkExistsOnDisk(ChunkID id) const;

    const WorldMetadata& getMetadata() const;
    bool updateMetadata(const WorldMetadata& meta);

    static std::vector<std::string> listWorlds(const std::string& base_dir = DEFAULT_WORLDS_DIR);
};

}
