#include "loaders/obj_loader.h"
#include "rendering/opengl/gl_mesh.h"
#include "voxel_engine/client/resource_manager.h"
#include <iostream>

namespace voxeng::client {

std::shared_ptr<Mesh> ResourceManager::getModel(const std::string& file_path) {
    return loadModel(file_path).mesh;
}

ResourceManager::ModelLoadResult ResourceManager::loadModel(const std::string& file_path) {
    auto it = m_meshes.find(file_path);
    if (it != m_meshes.end()) {
        // Mesh already cached — return it without material (texture already loaded by caller)
        return {it->second, {}};
    }

    auto model_opt = ObjLoader::load(file_path);
    if (!model_opt.has_value()) {
        std::cerr << "ResourceManager: Failed to load model " << file_path << "\n";
        return {nullptr, {}};
    }

    // Create OpenGL Mesh (TODO: in the future add other backends)
    auto gl_mesh = std::make_shared<GLMesh>();
    gl_mesh->upload(model_opt->mesh);
    m_meshes[file_path] = gl_mesh;

    return {gl_mesh, model_opt->material.diffuse_texture_path};
}

}
