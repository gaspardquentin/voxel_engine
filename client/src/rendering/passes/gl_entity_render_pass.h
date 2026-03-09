#include "rendering/interface/irender_pass.h"
#include "rendering/shader.h"
#include "voxel_engine/client/mesh.h"
#include "voxel_engine/client/resource_manager.h"

#include <memory>
#include <vector>

namespace voxeng::client {

class GLEntityRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  std::vector<std::shared_ptr<Mesh>> m_entities_meshes;
  ResourceManager m_resource_manager;

public:
  GLEntityRenderPass(const Shader& shader);

  void render(const Camera& camera) override;
};

} // namespace voxeng::client
