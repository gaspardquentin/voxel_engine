#include "rendering/interface/irender_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl_texture_array.h"
#include "rendering/shader.h"
#include "rendering/world_mesh_builder.h"
#include "voxel_engine/world.h"

#include <memory>
#include <unordered_map>

class GLWorldRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  const World& m_world;
  WorldMeshBuilder m_world_mesh_builder;

  std::unordered_map<ChunkID, std::shared_ptr<GLMesh>> m_chunk_meshes;
  OpenGLTextureArray m_textures;

public:
  GLWorldRenderPass(const Shader& shader, const World& world);

  void loadTextures();

  void render(const Camera& camera) override;


};
