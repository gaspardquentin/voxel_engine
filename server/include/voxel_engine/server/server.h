#pragma once

#include "voxel_engine/math_utils.h"
#include "voxel_engine/network/i_server_connection.h"
#include "voxel_engine/network/server_request.h"
#include "voxel_engine/user.h"
#include "voxel_engine/voxel_types.h"
#include <cstdint>
#include <memory>

namespace voxeng::server {

struct WorldConfig {
    const std::vector<VoxelType>& voxel_types = DEFAULT_VOXEL_TYPES;
    uint64_t seed = math::generate_seed();
};


class Server {
    class Impl;
    std::unique_ptr<Impl> m_impl;

public:
    Server(network::IServerConnection& connection);
    ~Server();
    void update(float delta_time);

    // Request handling
    void handleRequest(const network::PlayerVoxelRequest& req);
    void handleRequest(const network::PlayerPositionRequest& req);
    void handleRequest(const network::CreateWorldRequest& req);
    void handleRequest(const network::LoadWorldRequest& req);
    void handleRequest(const network::SaveWorldRequest& req);
    void handleRequest(const network::JoinWorldRequest& req);
    void handleRequest(const network::ListWorldsRequest& req);

    void saveAndCloseWorld();
    bool isWorldLoaded() const;
};

}
