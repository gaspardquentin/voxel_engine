#pragma once

#include "rendering/mesh_data.h"
#include "voxel_engine/mesh.h"
#include <GL/glew.h>

class GLMesh : public voxeng::Mesh {
  GLuint vao = 0, vbo = 0, ebo = 0;
  GLsizei index_count = 0;

public:
  GLMesh() = default;
  GLMesh(const GLMesh&) = delete;
  GLMesh& operator=(const GLMesh&) = delete;
  GLMesh(GLMesh&& o) noexcept { *this = std::move(o); }
  GLMesh& operator=(GLMesh&& o) noexcept {
    vao = o.vao; vbo = o.vbo; ebo = o.ebo; index_count = o.index_count;
    o.vao = o.vbo = o.ebo = 0; o.index_count = 0;
    return *this;
  }
  ~GLMesh() override {
    destroy();
  }

  inline void test() { std::cout << vao << std::endl; } //TODO: maybe remove this
  void upload(const MeshData& mesh_data);

  // Implementation of the generic interface
  void draw() const override;

  // OpenGL specific draw with mode control
  void draw(GLenum mode) const;

  void destroy();
};

