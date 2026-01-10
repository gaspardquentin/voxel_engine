#include "glui_render_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/ui_mesh_builder.h"

GLUIRenderPass::GLUIRenderPass(const Shader& shader): m_shader_prog(shader) {
    m_crosshair_mesh.upload(m_ui_mesh_builder.buildCrosshair());
}

void GLUIRenderPass::render(const Camera& camera) {
    glDisable(GL_DEPTH_TEST);
    m_shader_prog.bind();

    m_crosshair_mesh.draw(GL_LINES);

    m_shader_prog.unbind();
    glEnable(GL_DEPTH_TEST);
}
