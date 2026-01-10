#include "rendering/interface/irender_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/shader.h"
#include "rendering/world_mesh_builder.h"

#include <memory>
#include <vector>

class GLWorldRenderPass: public IRenderPass {
  WorldMeshBuilder m_world_mesh_builder;
  const Shader& m_shader_prog;
  const World& m_world;
  //std::vector<std::unique_ptr<GLMesh>> chunk_meshes;
  std::vector<GLMesh> m_chunk_meshes;

public:
  GLWorldRenderPass(const Shader& shader, const World& world);

  void addChunkMesh(GLMesh mesh);
  void loadTextures(World& world);
  void initChunkMeshes(int chunk_nbr);

  void render(const Camera& camera) override;


};
