#pragma once

/*
	* This file describes the binary format used
	* to save worlds in this voxel engine.
	* author: Gaspard QUENTIN
*/

#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/user.h"
#include "voxel_engine/voxel_types.h"
#include <array>
#include <cstdint>
#include <vector>


#define FORMAT_VERSION 1
#define FORMAT_SUB_VERSION 1


namespace voxeng {

struct PlayerSaveData {
	UserID uid;
	WorldCoord position;
};

struct WorldMetadata {
	uint16_t format_version = FORMAT_VERSION;
	uint16_t format_sub_version = FORMAT_SUB_VERSION;
	uint64_t world_seed = 0;
	std::vector<VoxelType> voxel_types = DEFAULT_VOXEL_TYPES;
	std::vector<PlayerSaveData> known_players;
};

struct ChunkSaveData {
	Vec3f chunk_pos; //note: maybe y position not needed
	// std::vector<VoxelID> ids_used; note: use this when we scale the number of voxel types
	std::array<VoxelID, CHUNK_SIZE> voxels = {0};
};



// Voxel types are now stored in WorldMetadata (since format 1.1).
// When loading older saves (sub_version 0), fall back to DEFAULT_VOXEL_TYPES.

// Note for scaling in the future :
//
// When there will be more than 255 blocs type, we will need voxelID to be a bigger type
// Then, to save space for each, we will create an std::vector<VoxelID> id_used field on the chunk_header
// so that the value stored on the voxel remains a uint8_t, which then will represent the index on the id_used table.

}
