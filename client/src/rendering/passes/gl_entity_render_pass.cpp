#include "gl_entity_render_pass.h"
#include "voxel_engine/types.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

namespace voxeng::client {

GLEntityRenderPass::GLEntityRenderPass(const Shader& shader, const std::optional<ClientWorld>& world):
    m_shader_prog(shader), m_world(world) {
    /*
    auto cow = m_resource_manager.getModel(VOXEL_ENGINE_ASSETS_DIR "/models/cow.obj");
    if (cow) m_entities_meshes.push_back(cow);

    auto teapot = m_resource_manager.getModel(VOXEL_ENGINE_ASSETS_DIR "/models/teapot.obj");
    if (teapot) m_entities_meshes.push_back(teapot);
    */
}

void GLEntityRenderPass::render(const Camera& camera) {
    if (!m_world.has_value()) return;
    m_shader_prog.bind();

    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 view = camera.getViewMatrix();
    
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    
    // Set a neutral gray color
    float color[] = {0.6f, 0.6f, 0.6f, 1.0f}; 
    m_shader_prog.setUniformVec4("color", color);

    /*
    float x_pos = 50.0f;
    int i = 0;
    for (auto& mesh: m_entities_meshes) {
        glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(x_pos * i, 13.0f, 0.0f));
        m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(current_model));
        mesh->draw();
        i++;
    }
    */


    for (auto& [ent_id, ent]: m_world->getEntities()) {
        auto& mesh = m_entities_meshes[ent_id];
        if (!mesh) {
            //TODO: maybe add support for other model types than OBJ
            mesh = m_resource_manager.getModel(VOXEL_ENGINE_ASSETS_DIR "/models/" + ent.model_name + ".obj");
        }
        //if (ent.update_dirty) {} TODO: I don't know if we need this update dirty but in case
        glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(ent.position.x, ent.position.y, ent.position.z));
        m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(current_model));
        mesh->draw();
    }


    m_shader_prog.unbind();
}

}
