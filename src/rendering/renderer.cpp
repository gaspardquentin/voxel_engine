#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "voxel_engine/voxel_types.h"
#include "voxel_engine/chunk.h"
#include "opengl_texture.h"

void Renderer::render(World& world, const Camera& camera) {
    m_shader_prog.bind();

    // MVP matrices calc
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
	glm::radians(camera.getZoom()),
	(float) m_screen_width / (float) m_screen_height,
	0.1f,
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
	ChunkRenderer& cr = m_chunk_renderers[c.getRendererId()];
	if (c.update()) {
	    MeshData md = m_mesh_builder.buildMesh(c);
	    cr.upload(md);
	}

	cr.draw();

    }
    m_shader_prog.unbind();

    // Crosshair rendering
    Shader crosshair_prog(
	VOXEL_ENGINE_SHADER_DIR "/crosshair.vert",
	VOXEL_ENGINE_SHADER_DIR "/crosshair.frag"
    );

    float crosshairVertices[] = {
	-0.01f,  0.0f, 0.0f,
	0.01f,  0.0f, 0.0f,
	0.0f, -0.015f, 0.0f,
	0.0f,  0.015f, 0.0f
    };

    GLuint crosshairVAO, crosshairVBO;
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    crosshair_prog.bind();  // simple shader
    glBindVertexArray(crosshairVAO);
    glDisable(GL_DEPTH_TEST);  // always on top
    glDrawArrays(GL_LINES, 0, 4);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    crosshair_prog.unbind();  // simple shader


  }


void Renderer::loadTextures(World& world) {
    int voxel_types_nbr = world.getVoxelTypes().size();
    for (int i = 1; i < voxel_types_nbr; i++) {
	const Texture& old_texture = *(world.getVoxelType(i).getTexture());
	auto glt = std::make_shared<OpenGLTexture>(old_texture);
	world.setTextureForType(i, glt);
	glt->loadTexture();
	glt->bind(); //TODO: move this away
    }
}


void Renderer::generateChunkRenderers(int chunk_nbr) {
    for (int i = 0; i < chunk_nbr ; i++) {
	m_chunk_renderers.push_back({});
    }
}

void Renderer::destroy() {
    for (ChunkRenderer& cr: m_chunk_renderers) {
	cr.destroy();
    }
}

void ChunkRenderer::upload(const MeshData& mesh_data) {
    index_count = static_cast<GLsizei>(mesh_data.indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
		 mesh_data.vertices.size() * sizeof(Vertex),
		 mesh_data.vertices.data(),
		 GL_STATIC_DRAW);

    // index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 mesh_data.indices.size() * sizeof(uint32_t),
		 mesh_data.indices.data(),
		 GL_STATIC_DRAW);

    // vertex layout
    GLuint stride = sizeof(Vertex);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, pos));
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, normal));
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));

    //glBindBuffer(GL_ARRAY_BUFFER, 0); TODO: what is this ?
    glBindVertexArray(0);

}

void ChunkRenderer::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void ChunkRenderer::destroy() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}
