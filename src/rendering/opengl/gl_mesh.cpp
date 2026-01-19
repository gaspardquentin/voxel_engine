#include "rendering/opengl/gl_mesh.h"

void GLMesh::upload(const MeshData& mesh_data) {
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
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, uv));

    //glBindBuffer(GL_ARRAY_BUFFER, 0); TODO: what is this ?
    glBindVertexArray(0);

}

void GLMesh::draw(GLenum mode) const {
    glBindVertexArray(vao);
    glDrawElements(mode, index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void GLMesh::destroy() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}
