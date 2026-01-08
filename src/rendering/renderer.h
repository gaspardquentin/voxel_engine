#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <string>

#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl/opengl_render_pass.h"
#include "voxel_engine/world.h"
#include "shader.h"
#include "camera.h"
#include "mesh_builder.h"

class UIRenderPass: public GLRenderPass {
public:
  void upload(const MeshData& mesh_data) override;
};

class RenderPipeline {
  Shader m_shader_prog;
  MeshBuilder m_mesh_builder;
  std::vector<GLMesh> m_chunk_renderers;
  unsigned int m_screen_width, m_screen_height;

public:
  RenderPipeline(std::string vertex_shader_path,
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

