#include "glworld_render_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl_texture.h"
#include "rendering/world_mesh_builder.h"
#include "voxel_engine/voxel_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace voxeng::client {

GLWorldRenderPass::GLWorldRenderPass(const Shader& shader,
				     const std::optional<ClientWorld>& world)
    : m_shader_prog(shader), m_world(world) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(77.0f/255.0f, 109.0f/255.0f, 157.0f/255.0f, 1.0f);
}

void GLWorldRenderPass::loadTextures(const std::vector<VoxelType>& voxel_types) {
    m_textures.loadTextures(voxel_types);
    m_textures.bind();
}

void GLWorldRenderPass::render(const Camera& camera) {
    if (!m_world.has_value()) return;

    glEnable(GL_DEPTH_TEST);
    m_shader_prog.bind();

    // MVP matrices calc
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix();

    // set shader MVP uniforms
    m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(model));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));

    // voxel texture TODO: change approach
    m_shader_prog.setUniform1i("uTextures", 0);
    for (auto& [chunk_id, c]: m_world->getChunks()) {
        auto& mesh = m_chunk_meshes[chunk_id];
        if (!mesh) {
            mesh = std::make_shared<GLMesh>();
        }
        if (c.isRenderDirty()) {
            MeshData md = m_world_mesh_builder.buildMesh(c);
            mesh->upload(md);
            c.clearRenderDirty();
        }
        mesh->draw();
    }

    // Remove stale mesh entries for unloaded chunks (throttled to once per second)
    auto now = std::chrono::steady_clock::now();
    if (now - m_last_cleanup >= std::chrono::seconds(1)) {
        m_last_cleanup = now;
        const auto& active_chunks = m_world->getChunks();
        for (auto it = m_chunk_meshes.begin(); it != m_chunk_meshes.end(); ) {
            if (active_chunks.find(it->first) == active_chunks.end()) {
                it = m_chunk_meshes.erase(it);
            } else {
                ++it;
            }
        }
    }

    m_shader_prog.unbind();
}

}
