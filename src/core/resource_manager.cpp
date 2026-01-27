#include "voxel_engine/resource_manager.h"
#include "loaders/obj_loader.h"
#include "rendering/opengl/gl_mesh.h"
#include <iostream>

namespace voxeng {

std::shared_ptr<Mesh> ResourceManager::getModel(const std::string& file_path) {
    auto it = m_meshes.find(file_path);
    if (it != m_meshes.end()) {
        return it->second;
    }

    auto mesh_data_opt = ObjLoader::load(file_path);
    if (!mesh_data_opt.has_value()) {
        std::cerr << "ResourceManager: Failed to load model " << file_path << "\n";
        return nullptr;
    }

    // Create OpenGL Mesh (TODO: in the future add other backends)
    auto gl_mesh = std::make_shared<GLMesh>();
    gl_mesh->upload(mesh_data_opt.value());
    m_meshes[file_path] = gl_mesh;

    return gl_mesh;
}

}
