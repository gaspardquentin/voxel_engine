#pragma once

#include "voxel_engine/client/mesh.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace voxeng::client {

struct ModelData;

class ResourceManager {
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;

public:
    ResourceManager() = default;

    // Gets a model from cache or loads it if not present.
    // Currently assumes OpenGL backend. TODO: implement other backends and backend-switching
    std::shared_ptr<Mesh> getModel(const std::string& file_path);

    // Loads a model with its material data (mesh + texture paths).
    // Returns nullptr in the ModelData if loading fails.
    struct ModelLoadResult {
        std::shared_ptr<Mesh> mesh;
        std::string diffuse_texture_path;
    };
    ModelLoadResult loadModel(const std::string& file_path);
};

}
