#pragma once

#include "rendering/mesh_data.h"

class UIMeshBuilder {
  //TODO: find a better non-const approach (so that crosshair can be changed in game)
  const MeshData crosshair = {
    {
      // vertices
      {{-0.01f,  0.0f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{0.01f,  0.0f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{0.0f,  -0.015f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
      {{0.0f,  0.015f, 0.0f,}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    },
    // indices
    {0, 1, 2, 3}
  };

public:
  MeshData buildCrosshair() const;
};
