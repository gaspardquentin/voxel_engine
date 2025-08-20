#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <string>

#include "voxel_engine/world.h"
#include "shader.h"
#include "camera.h"
#include "mesh_builder.h"

class ChunkRenderer {
  GLuint vao = 0, vbo = 0, ebo = 0;
  GLsizei index_count = 0;

public:
  ChunkRenderer() = default;
  ChunkRenderer(const ChunkRenderer&) = delete;
  ChunkRenderer& operator=(const ChunkRenderer&) = delete;
  ChunkRenderer(ChunkRenderer&& o) noexcept { *this = std::move(o); }
  ChunkRenderer& operator=(ChunkRenderer&& o) noexcept {
    vao = o.vao; vbo = o.vbo; ebo = o.ebo; index_count = o.index_count;
    o.vao = o.vbo = o.ebo = 0; o.index_count = 0;
    return *this;
  }
  ~ChunkRenderer() {}

  inline void test() { std::cout << vao << std::endl; }
  void upload(const MeshData& mesh_data);
  void draw() const;
  void destroy();
};

class Renderer {
  Shader m_shader_prog;
  MeshBuilder m_mesh_builder;
  std::vector<ChunkRenderer> m_chunk_renderers;
  unsigned int m_screen_width, m_screen_height;

public:
  Renderer(std::string vertex_shader_path,
           std::string fragment_shader_path,
           unsigned int screen_width,
           unsigned int screen_height):
    m_shader_prog(vertex_shader_path, fragment_shader_path),
    m_screen_width(screen_width), m_screen_height(screen_height)
  {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //glClearColor(108.0f/255.0f, 188.0f/255.0f, 211.0f/255.0f, 0.0f);
    glClearColor(77.0f/255.0f, 109.0f/255.0f, 157.0f/255.0f, 1.0f);
  }
  void render(World& world, const Camera& camera);
  void loadTextures(World& world);
  void generateChunkRenderers(int chunk_nbr);
  void destroy();
};

