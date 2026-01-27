#include "glworld_render_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl_texture.h"
#include "rendering/world_mesh_builder.h"
#include "voxel_engine/voxel_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLWorldRenderPass::GLWorldRenderPass(const Shader& shader,
				     const World& world)
    : m_shader_prog(shader), m_world(world), m_textures(world.getVoxelTypes()) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(77.0f/255.0f, 109.0f/255.0f, 157.0f/255.0f, 1.0f);
}

void GLWorldRenderPass::addChunkMesh(std::shared_ptr<GLMesh> mesh) {
    m_chunk_meshes.push_back(std::move(mesh));
}

void GLWorldRenderPass::loadTextures() {
    m_textures.loadTextures();
    m_textures.bind();
}

void GLWorldRenderPass::initChunkMeshes(int chunk_nbr) {
    for (int i = 0; i < chunk_nbr ; i++) {
        addChunkMesh(std::make_shared<GLMesh>());
    }
}

void GLWorldRenderPass::render(const Camera& camera) {
    if (m_chunk_meshes.empty()) {
        return;
    }
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& [_, c]: m_world.getChunks()) {
        auto& cr = m_chunk_meshes[c.getRendererId()];
        if (c.update()) {
            MeshData md = m_world_mesh_builder.buildMesh(c);
            cr->upload(md);
        }

        cr->draw();
    }

    m_shader_prog.unbind();
}
