#include "client_world.h"
#include "rendering/interface/irender_pass.h"
#include "rendering/shader.h"
#include "voxel_engine/client/mesh.h"
#include "voxel_engine/client/resource_manager.h"
#include "voxel_engine/types.h"

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace voxeng::client {

class GLEntityRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  const std::optional<ClientWorld>& m_world;
  std::unordered_map<EntityID, std::shared_ptr<Mesh>> m_entities_meshes;
  ResourceManager m_resource_manager;

public:
  GLEntityRenderPass(const Shader& shader, const std::optional<ClientWorld>& world);

  void render(const Camera& camera) override;
};

}
