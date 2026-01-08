#include "glchunk_render_pass.h"
#include "rendering/opengl/gl_mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLChunkRenderPass::GLChunkRenderPass(const Shader& shader, float screen_width, float screen_height)
    : m_shader_prog(shader), m_screen_width(screen_width), m_screen_height(screen_height) {}

void GLChunkRenderPass::render(const World& world, const Camera& camera) {
    m_shader_prog.bind();

    
    // MVP matrices calc
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
	glm::radians(camera.getZoom()),
	(float) m_screen_width / (float) m_screen_height,
	0.1f, //TODO: move those magic values into something
	3000.0f
    );

    // set shader MVP uniforms
    m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(model));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));

    // voxel texture TODO: change approach
    m_shader_prog.setUniform1i("atlas", 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (Chunk& c: world.getChunks()) {
	GLMesh& cr = m_chunk_renderers[c.getRendererId()];
	if (c.update()) {
	    MeshData md = m_mesh_builder.buildMesh(c);
	    cr.upload(md);
	}

	cr.draw();

    }


	
    m_shader_prog.unbind();
}
