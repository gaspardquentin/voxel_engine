#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "voxel_engine/voxel_types.h"

void Renderer::render(const World& world, const Camera& camera) {
    m_shader_prog.bind();

    /*
    glm::mat4 view = glm::lookAt(
	glm::vec3(10.0f, 10.0f, 30.0f), // Move back and up so we see the chunk
	glm::vec3(12.0f, 2.0f, 8.0f),   // Look at the chunk's center
	glm::vec3(0.0f, 1.0f, 0.0f)
    );
    */

    // MVP matrices calc
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(
	glm::radians(camera.getZoom()),            // Wider FOV to be sure we see it
	(float) m_screen_width / (float) m_screen_height,
	0.1f,
	1000.0f                         // Big far plane
    );

    // set shader MVP uniforms
    m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(model));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));

    //std::vector<const Chunk&> to_render;
    // get the renderable chunks
    //TODO: implement this
    // get the mesh data of the chunks to render
    //std::vector<MeshData> meshes;
    //for (const Chunk& c: to_render) {
	//meshes.push_back(m_mesh_builder.buildMesh(c));
    //}
    // TODO: handle camera and shaders
    std::vector<Chunk> chunks;
    chunks.push_back({DEFAULT_VOXEL_TYPES});
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 voxel_color(1.0f, 0.5f, 0.2f);
    glm::vec3 outline_color(0.0f, 0.0f, 0.2f);
    m_shader_prog.setUniformVec3("objectColor", glm::value_ptr(voxel_color));
    m_shader_prog.setUniformVec3("outlineColor", glm::value_ptr(outline_color));
    for (const Chunk& c: chunks) {
	ChunkRenderer& cr = m_chunk_renderers[c.getRendererId()];
	MeshData md = m_mesh_builder.buildMesh(c);
	cr.upload(md);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	//cr.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(4.0f); // Set line thickness
	cr.draw();
    }
    m_shader_prog.unbind();
    //m_chunk_renderers[chunks[0].getRendererId()].destroy();
    //exit(0);
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
