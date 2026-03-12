#pragma once

#include "voxel_engine/callbacks.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/message.h"
#include "voxel_engine/voxel_types.h"
#include <chrono>
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
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
};

// receives the list of worlds paths
struct ListWorldsEvent {
    std::vector<std::string> paths;
};

struct NewWorldCreatedEvent {
    std::string path;
};

// Single message broadcast (pushed on each new message)
struct ChatMessageEvent {
    Message message;
};

// Batch of messages sent once on world join
struct ChatHistoryEvent {
    std::vector<Message> messages;
};

using ClientEvent = std::variant<
    WorldLoadEvent,
    ChunkDataEvent,
    ChunkUnloadEvent,
    ServerErrorEvent,
    ListWorldsEvent,
    NewWorldCreatedEvent,
    VoxelChangedEvent,
    ChatMessageEvent,
    ChatHistoryEvent
>;

}
