#include "rendering/interface/irender_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl_texture_array.h"
#include "rendering/shader.h"
#include "rendering/world_mesh_builder.h"

#include <memory>
#include <vector>

class GLWorldRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  const World& m_world;
  WorldMeshBuilder m_world_mesh_builder;

  std::vector<std::shared_ptr<GLMesh>> m_chunk_meshes;
  OpenGLTextureArray m_textures;

public:
  GLWorldRenderPass(const Shader& shader, const World& world);

  void loadTextures();
  void addChunkMesh(std::shared_ptr<GLMesh> mesh);
  void initChunkMeshes(int chunk_nbr);

  void render(const Camera& camera) override;


};
