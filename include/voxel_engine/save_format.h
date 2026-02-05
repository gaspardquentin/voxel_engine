#pragma once

/*
	* This file describes the binary format used 
	* to save worlds in this voxel engine.
	* Format version: 0.1
	* author: Gaspard QUENTIN
*/

#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/voxel_types.h"
#include <array>
#include <cstdint>
#include <vector>


#define FORMAT_VERSION 0
#define FORMAT_SUB_VERSION 1


struct VoxelSaveData {
	VoxelID type;
	//uint8_t type; note: use this when we scale
};

struct ChunkSaveData {
	Vec3f chunk_pos; //note: maybe y position not needed
	// std::vector<VoxelID> ids_used; note: use this when we scale the number of voxel types
	std::array<VoxelSaveData, CHUNK_SIZE> voxels = {0}; 
	//TODO: IMPORTANT : since we are not generating chunks on the y-axis, what should we do ? 
	// get rid of the chunk_height ? But then how do we calculate the number of voxels to save per chunk ?
};

struct WorldSaveData {
	uint16_t format_version = FORMAT_VERSION;
	uint16_t format_sub_version = FORMAT_SUB_VERSION;
	uint64_t world_seed;
	uint64_t chunks_nbr;
	std::vector<ChunkSaveData> chunks;
};

// Note for scaling in the future :
//
// When there will be more than 255 blocs type, we will need voxelID to be a bigger type
// Then, to save space for each, we will create an std::vector<VoxelID> id_used field on the chunk_header
// so that the value stored on the voxel remains a uint8_t, which then will represent the index on the id_used table.
