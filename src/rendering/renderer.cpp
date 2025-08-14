#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "voxel_engine/voxel_types.h"

void Renderer::render(const World& world, const Camera& camera) {
    m_shader_prog.bind();
    /*
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(
	glm::vec3(4.0f, 0.0f, 5.0f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f)
    );


    float zoom = 45.0f;
    glm::mat4 projection = glm::perspective(
      glm::radians(zoom),
      800.0f / 600.0f,
      0.1f,
      100.0f
    );
    */
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::lookAt(
	glm::vec3(10.0f, 20.0f, 20.0f), // Move back and up so we see the chunk
	glm::vec3(12.0f, 2.0f, 8.0f),   // Look at the chunk's center
	glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection = glm::perspective(
	glm::radians(70.0f),            // Wider FOV to be sure we see it
	800.0f / 600.0f,
	0.1f,
	1000.0f                         // Big far plane
    );


    m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(model));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));

    // TODO: camera m_shader_prog.setUniformMatrix4fv(const std::string &name, const float *matrix)
    
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
    for (const Chunk& c: chunks) {
	ChunkRenderer& cr = m_chunk_renderers[c.getRendererId()];
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f); // Set line thickness
	MeshData md = m_mesh_builder.buildMesh(c);
	/*
	for (const Vertex& v: md.vertices) {
	    std::cout << v.pos << " ";
	}
	*/
	//std::cout << std::endl;
	cr.upload(md);
	//if (m_shader_prog.validateProgram()) {
	    cr.draw();
	//}
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
