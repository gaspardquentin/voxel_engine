#pragma once

// Interface for rendering parts of the engine (UI, chunks...)
// This interface is common to every rendering method.
#include "voxel_engine/client/camera.h"

namespace voxeng::client {

class IRenderPass {
public:
  virtual void render(const Camera& camera) = 0;
};

} // namespace voxeng::client
