#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include "voxel_engine/math_utils.h"

namespace voxeng::client {

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

struct MaterialData {
  std::string diffuse_texture_path;
  Vec3 diffuse_color = {1.f, 1.f, 1.f};
};

struct ModelData {
  MeshData mesh;
  MaterialData material;
};

}


