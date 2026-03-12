#pragma once

#include "voxel_engine/callbacks.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/user.h"
#include "voxel_engine/voxel_types.h"
#include <cstdint>
#include <variant>

/*
 * Server Requests
 *
 * Requests are the messages from the client to the server. client -> server
 *
*/

namespace voxeng::network {


//TODO: in requests like create/load world and join add information regarding the user trying to connect

struct PlayerVoxelRequest {
    UserID user;
    Vec3f position; //TODO: maybe use vec3i instead ???
    VoxelID voxel;
};

struct PlayerPositionRequest {
    UserID user;
    Vec3f position;
};

// asks for the list of available worlds
struct ListWorldsRequest {};

struct CreateWorldRequest {
    std::string name;
};

// Note: only when a LocalConnection is set to the server. (so when the user is the one hosting the world)
struct LoadWorldRequest {
    std::string world_path; //TODO: are you sure it's that ?
};


struct SaveWorldRequest {};

struct JoinWorldRequest {
    // std::string world_name or maybe an idea ; TODO: maybe use this when/if we implement multiverses
    UserProfile user;
    uint8_t render_distance;
};


struct SendChatRequest {
    UserProfile user;
    std::string content;
};

using ServerRequest = std::variant<
    PlayerVoxelRequest,
    PlayerPositionRequest,
    CreateWorldRequest,
    LoadWorldRequest,
    SaveWorldRequest,
    ListWorldsRequest,
    JoinWorldRequest,
    SendChatRequest
>;

}
