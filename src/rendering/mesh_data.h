#pragma once

#include <vector>
#include <stdint.h>

#include "voxel_engine/math_utils.h"

struct Vertex {
  Vec3 pos, normal;
  Vec3 uv; //TODO: maybe transform it into vec2 and add a int layerIndex to the struct
};

struct MeshData {
  std::vector<Vertex> vertices;   // positions, normals, texcoords, etc.
  std::vector<uint32_t> indices;
  inline void clear() { 
    vertices.clear();
    indices.clear();
  }
};


