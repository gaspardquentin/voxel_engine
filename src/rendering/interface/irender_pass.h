#pragma once

// Interface for rendering parts of the engine (UI, chunks...)
// This interface is common to every rendering method.
#include "rendering/camera.h"
class IRenderPass {
public:
  virtual void render(const World& world, const Camera& camera);
};
