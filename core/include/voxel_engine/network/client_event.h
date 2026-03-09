#pragma once

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

struct WorldLoadEvent {
    std::vector<VoxelType> voxel_types;
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
