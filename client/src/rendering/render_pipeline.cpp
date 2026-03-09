#include "render_pipeline.h"

namespace voxeng::client {

void RenderPipeline::render(const Camera& camera) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& pass: m_render_passes) {
	pass->render(camera);
    }
}

} // namespace voxeng::client

