#pragma once

#include "rendering/mesh_data.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/math_utils.h"

enum class VoxelFace: uint8_t { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };

//TODO: maybe rename this to ChunkMeshBuilder ??
class WorldMeshBuilder {
  void emitFace(MeshData& mesh_data, const Chunk& chunk, ChunkCoord voxel_pos, VoxelFace face, const VoxelType& voxel) const;

public:
  MeshData buildMesh(const Chunk& chunk) const;
};

