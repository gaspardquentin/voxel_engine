#pragma once

#include "rendering/mesh_data.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/math_utils.h"

#include <array>
#include <vector>

namespace voxeng::client {

enum class VoxelFace: uint8_t { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };

//TODO: maybe rename this to ChunkMeshBuilder ??
class WorldMeshBuilder {
  // lookup tables indexed by VoxelID, filled by setVoxelTypes()
  std::array<uint8_t, 256> m_is_air;
  std::array<uint8_t, 256> m_transparent;

  void emitFace(MeshData& mesh_data, Vec3f voxel_world_pos, VoxelFace face, VoxelID voxel) const;

public:
  WorldMeshBuilder();

  void setVoxelTypes(const std::vector<VoxelType>& voxel_types);

  MeshData buildMesh(const voxeng::Chunk& chunk) const;
  // Only reads the given data, safe to call from a worker thread on a snapshot
  MeshData buildMesh(const std::array<VoxelID, CHUNK_SIZE>& data, Vec3f world_pos) const;
};

}

