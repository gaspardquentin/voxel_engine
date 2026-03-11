#pragma once

#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/voxel_types.h"
#include <variant>
#include <vector>

/*
 * Client Events
 *
 * Events are the messages from the server to the client. server -> client
 *
*/


namespace voxeng::network {

//TODO: maybe rename world joined ??
struct WorldLoadEvent {
    std::vector<VoxelType> voxel_types;
    WorldCoord spawn_pos = {0.0f, 12.0f, -5.0f}; // default spawn pos
};

struct ChunkDataEvent {
    ChunkID id;
    std::array<VoxelID, Chunk::SIZE> data;
};

struct ChunkUnloadEvent {
    ChunkID id;
};

struct VoxelChangedEvent {
    ChunkID chunk_id;
    ChunkCoord chunk_pos;
    VoxelID new_voxel;
};

struct ServerErrorEvent {
    std::string kind;
    std::string message;
};

// receives the list of worlds paths
struct ListWorldsEvent {
    std::vector<std::string> paths;
};

struct NewWorldCreatedEvent {
    std::string path;
};

using ClientEvent = std::variant<
    WorldLoadEvent,
    ChunkDataEvent,
    ChunkUnloadEvent,
    ServerErrorEvent,
    ListWorldsEvent,
    NewWorldCreatedEvent,
    VoxelChangedEvent
>;

}
