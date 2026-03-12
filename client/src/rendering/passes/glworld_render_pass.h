#include "client_world.h"
#include "rendering/interface/irender_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl_texture_array.h"
#include "rendering/shader.h"
#include "rendering/world_mesh_builder.h"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace voxeng::client {

class GLWorldRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  const std::optional<ClientWorld>& m_world;
  WorldMeshBuilder m_world_mesh_builder;

  std::unordered_map<ChunkID, std::shared_ptr<GLMesh>> m_chunk_meshes;
  OpenGLTextureArray m_textures;
  std::chrono::steady_clock::time_point m_last_cleanup = std::chrono::steady_clock::now();

public:
  GLWorldRenderPass(const Shader& shader, const std::optional<ClientWorld>& world);

  void loadTextures(const std::vector<VoxelType>& voxel_types);

  void render(const Camera& camera) override;
};

}
