#include "gl_entity_render_pass.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

GLEntityRenderPass::GLEntityRenderPass(const Shader& shader): m_shader_prog(shader) {
    auto cow = m_resource_manager.getModel(VOXEL_ENGINE_ASSETS_DIR "/models/cow.obj");
    if (cow) m_entities_meshes.push_back(cow);

    auto teapot = m_resource_manager.getModel(VOXEL_ENGINE_ASSETS_DIR "/models/teapot.obj");
    if (teapot) m_entities_meshes.push_back(teapot);
}

void GLEntityRenderPass::render(const Camera& camera) {
    m_shader_prog.bind();

    glm::mat4 projection = camera.getProjectionMatrix();
    glm::mat4 view = camera.getViewMatrix();
    
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    
    // Set a neutral gray color
    float color[] = {0.6f, 0.6f, 0.6f, 1.0f}; 
    m_shader_prog.setUniformVec4("color", color);

    float x_pos = 50.0f;
    int i = 0;
    for (auto& mesh: m_entities_meshes) {
        glm::mat4 current_model = glm::translate(glm::mat4(1.0f), glm::vec3(x_pos * i, 13.0f, 0.0f));
        m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(current_model));
        mesh->draw();
        i++;
    }
    m_shader_prog.unbind();
}