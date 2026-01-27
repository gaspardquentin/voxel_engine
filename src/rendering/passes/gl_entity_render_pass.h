#include "rendering/interface/irender_pass.h"
#include "rendering/shader.h"
#include "voxel_engine/mesh.h"
#include "voxel_engine/resource_manager.h"

#include <memory>
#include <vector>

class GLEntityRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  std::vector<std::shared_ptr<voxeng::Mesh>> m_entities_meshes;
  voxeng::ResourceManager m_resource_manager;

public:
  GLEntityRenderPass(const Shader& shader);

  void render(const Camera& camera) override;
};
