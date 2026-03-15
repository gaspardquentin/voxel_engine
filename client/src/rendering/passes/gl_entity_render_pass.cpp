#include "gl_entity_render_pass.h"
#include "voxel_engine/types.h"

#include <GL/glew.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace voxeng::client {

GLEntityRenderPass::GLEntityRenderPass(const Shader& shader, const std::optional<ClientWorld>& world):
    m_shader_prog(shader), m_world(world) {
}

void GLEntityRenderPass::render(const Camera& camera) {
    if (!m_world.has_value()) return;
    m_shader_prog.bind();

    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 view = camera.getViewMatrix();

    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));

    // Default fallback color
    float color[] = {0.6f, 0.6f, 0.6f, 1.0f};
    m_shader_prog.setUniformVec4("uColor", color);
    m_shader_prog.setUniform1i("uTexture", 0);

    for (auto& [ent_id, ent]: m_world->getEntities()) {
        auto& resource = m_entities[ent_id];
        if (!resource.mesh) {
            auto result = m_resource_manager.loadModel(VOXEL_ENGINE_ASSETS_DIR "/models/" + ent.model_name + ".obj");
            resource.mesh = result.mesh;
            if (!result.diffuse_texture_path.empty()) {
                auto tex = std::make_unique<OpenGLTexture>(Texture(result.diffuse_texture_path));
                tex->loadTexture();
                resource.texture = std::move(tex);
            }
        }

        glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(ent.position.x, ent.position.y, ent.position.z));
        m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(current_model));

        if (resource.texture) {
            m_shader_prog.setUniform1i("uHasTexture", 1);
            glActiveTexture(GL_TEXTURE0);
            resource.texture->bind();
        } else {
            m_shader_prog.setUniform1i("uHasTexture", 0);
        }

        resource.mesh->draw();
    }

    m_shader_prog.unbind();
}

}
