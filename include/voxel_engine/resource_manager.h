#pragma once

#include "voxel_engine/mesh.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace voxeng {

class ResourceManager {
public:
    ResourceManager() = default;

    // Gets a model from cache or loads it if not present.
    // Currently assumes OpenGL backend. TODO: implement other backends and backend-switching
    std::shared_ptr<Mesh> getModel(const std::string& file_path);

private:
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
};

}
