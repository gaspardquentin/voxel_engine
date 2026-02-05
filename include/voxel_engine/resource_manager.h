#pragma once

#include "voxel_engine/mesh.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace voxeng {

class ResourceManager {
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;

public:
    ResourceManager() = default;

    // Gets a model from cache or loads it if not present.
    // Currently assumes OpenGL backend. TODO: implement other backends and backend-switching
    std::shared_ptr<Mesh> getModel(const std::string& file_path);
    template<typename T>
    std::shared_ptr<T> get(const std::string& file_path);
};

}
