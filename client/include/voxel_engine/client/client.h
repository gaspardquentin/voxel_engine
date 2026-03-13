#pragma once

#include "voxel_engine/callbacks.h"
#include "voxel_engine/client/camera.h"
#include "voxel_engine/message.h"
#include "voxel_engine/network/client_event.h"
#include "voxel_engine/network/i_client_connection.h"
#include "voxel_engine/user.h"
#include "voxel_engine/voxel_types.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace voxeng::client {

// Camera configuration
/* TODO: maybe use this instead of config.h directory
struct CameraConfig {
    float default_cam_yaw = -90.0f;
    float default_cam_pitch = 0.0f;
    float camera_mov_speed = 5.0f;
    float mouse_sensitivity = 0.1f;
    float default_camera_zoom = 70.0f;
};
*/


enum class TextureMode { TEXTURE_ARRAY_2D, TEXTURE_ATLAS };

// Client configuration
struct ClientConfig {
    //CameraConfig camera_config;
    bool debugUI = true;
    bool worldUI = true;
    bool chatUI = true;
    bool vsync = false;
    unsigned int width = 1280;
    unsigned int height = 720;
    TextureMode texture_mode = TextureMode::TEXTURE_ARRAY_2D;
    uint8_t default_render_distance = 12;
    std::string profile_file_path = "profile.txt";
    std::string world_vertex_shader = VOXEL_ENGINE_SHADER_DIR "/vertex.vert";
    std::string world_fragment_shader = VOXEL_ENGINE_SHADER_DIR "/fragment.frag";
    std::string ui_vertex_shader = VOXEL_ENGINE_SHADER_DIR "/crosshair.vert";
    std::string ui_fragment_shader = VOXEL_ENGINE_SHADER_DIR "/crosshair.frag";
};

class Client {
    class Impl;
    std::unique_ptr<Impl> m_impl;
public:
    Client(ClientConfig& config, network::IClientConnection& connection);
    ~Client();

    /* User profile — loaded automatically on construction, saved on changes. */
    const std::string& getUsername() const;
    void setUsername(std::string username);
    UserID getUserId() const;

    void setRenderDistance(uint8_t render_distance);
    uint8_t getRenderDistance() const;

    WorldCoord getPlayerPos() const;

    /* Input/Action handling */
    void moveCamera(float xoffset, float yoffset, bool constrain_pitch = true);
    void zoomCamera(float yoffset);

    void setPlayerSpeed(float speed);
    float getPlayerSpeed() const;

    void movePlayer(Movement mov, float delta_time);
    void placeVoxel(uint8_t max_reach, VoxelID voxel);
    void removeVoxel(uint8_t max_reach);

    void sendChatMessage(const std::string& content);
    std::vector<Message> getAllMessages() const;
    std::vector<Message> getRecentMessages(size_t count) const;

    void createWorld(const std::string& name);
    void loadWorld(const std::string& world_path);
    void saveWorld();
    //TODO: implement a DisconnectRequest/LeaveWorldRequest to notify the server
    // that the user has left (needed for future multiplayer support)
    void leaveWorld();
    //void joinWorld(const std::string& world_path); TODO: world path needed ?
    void joinWorld();


    //TODO: maybe change name ? To list know/available worlds or getWorldsPaths idk
    const std::vector<std::string>& listWorlds() const;
    //TODO: maybe rename
    bool isWorldOpen() const;

    const std::vector<VoxelType> *getWorldVoxelTypes() const;
    const VoxelType *getWorldVoxelType(VoxelID vid) const;

    /* Server events handling */
    void handleEvent(network::WorldLoadEvent& event);
    void handleEvent(network::ChunkDataEvent& event);
    void handleEvent(const network::ChunkUnloadEvent& event);
    void handleEvent(const network::ListWorldsEvent& event);
    void handleEvent(const network::NewWorldCreatedEvent& event);
    void handleEvent(const network::ServerErrorEvent& event);
    void handleEvent(const network::VoxelChangedEvent& event);
    void handleEvent(const network::ChatMessageEvent& event);
    void handleEvent(const network::ChatHistoryEvent& event);
    void handleEvent(const network::EntitySpawnEvent& event);
    void handleEvent(const network::EntityUpdateEvent& event);
    void handleEvent(const network::EntityDespawnEvent& event);

    void render();
    void update();
};

}
