#include "rendering/interface/irender_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/shader.h"
#include "rendering/ui_mesh_builder.h"


class GLUIRenderPass: public IRenderPass {
  const Shader& m_shader_prog;
  UIMeshBuilder m_ui_mesh_builder;
  GLMesh m_crosshair_mesh;

public:
  GLUIRenderPass(const Shader& shader);

  void render(const Camera& camera) override;

};
