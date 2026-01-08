#include "rendering/interface/irender_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/shader.h"
#include <memory>
#include <vector>

class GLChunkRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  //TODO: maybe move this elsewhere (maybe Camera) and add update options
  float m_screen_width, m_screen_height;
  //std::vector<std::unique_ptr<GLMesh>> chunk_meshes;
  std::vector<GLMesh> chunk_meshes;

public:
  GLChunkRenderPass(const Shader& shader, float screen_width, float screen_height);
  void render(const World& world, const Camera& camera) override;
};
