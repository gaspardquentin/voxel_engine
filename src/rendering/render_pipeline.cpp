#include "render_pipeline.h"

void RenderPipeline::render(const Camera& camera) {
    for (auto& pass: m_render_passes) {
	pass->render(camera);
    }
}

