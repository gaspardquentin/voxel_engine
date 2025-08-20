#pragma once

#include <vector>

#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/math_utils.h"
#include <iostream>

struct Vertex {
  Vec3 pos, normal;
  Vec2 uv;
};

struct MeshData {
  std::vector<Vertex> vertices;   // positions, normals, texcoords, etc.
  std::vector<uint32_t> indices;
  inline void clear() { 
    std::cout << "causing troubles ?" << std::endl;
    vertices.clear();
    std::cout << "No." << std::endl;
    indices.clear();
    std::cout << "No." << std::endl;
  }
};

enum class VoxelFace: uint8_t { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };

class MeshBuilder {
  void emitFace(MeshData& mesh_data, const Chunk& chunk, Vec3i voxel_pos, VoxelFace face, const VoxelType& voxel) const;

public:
  MeshData buildMesh(const Chunk& chunk) const;
};

