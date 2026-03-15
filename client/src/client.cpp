#include "voxel_engine/client/client.h"
#include "client_world.h"
#include "voxel_engine/client/camera.h"
#include "rendering/shader_manager.h"
#include "rendering/passes/glui_render_pass.h"
#include "rendering/passes/glworld_render_pass.h"
#include "rendering/passes/gl_entity_render_pass.h"
#include "rendering/render_pipeline.h"
#include "voxel_engine/chat_history.h"
#include "voxel_engine/message.h"
#include "voxel_engine/network/client_event.h"
#include "voxel_engine/network/i_client_connection.h"
#include "voxel_engine/network/server_request.h"
#include "voxel_engine/user.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <optional>

namespace voxeng::client {

const int TICK_RATE = 20;
static constexpr auto TICK_DURATION = std::chrono::milliseconds((1000/TICK_RATE));

//TODO: IMPORTANT: separate input handling from client class and create 
//interfaces to remove glfw hard coded dependency (create IWindow and IInputHandler)


class Client::Impl {
public:
    network::IClientConnection& m_connection;
    ClientConfig& m_config;
    std::optional<Camera> m_camera;
    ShaderManager m_shader_manager;
    RenderPipeline m_render_pipeline;
    std::optional<ClientWorld> m_world;
    uint8_t m_render_distance;
    std::vector<std::string> m_worlds_paths_cache;
    bool m_worlds_cache_loaded = false;
    UserProfile m_user;
    bool m_position_dirty = false;
    std::chrono::time_point<std::chrono::steady_clock> m_previous_tick = std::chrono::steady_clock::now();
    ChatHistory m_chat_history{256};
    //GUILayer m_gui_layer;
    //GLFWwindow *m_window; //TODO: remove hard-coded dependency

    Impl(ClientConfig& config, network::IClientConnection& connection):
        m_connection(connection),
        m_config(config),
        //m_camera({0.0f, 0.0f, 3.0f}, config.width, config.height), //TODO: remove camera init (camera should not exist until user is connected to a world)
        m_render_distance(config.default_render_distance)
    {
        m_user = loadProfile();

        m_shader_manager.load("world", config.world_vertex_shader, config.world_fragment_shader);
        m_shader_manager.load("ui", config.ui_vertex_shader, config.ui_fragment_shader);
        m_shader_manager.load("entity", VOXEL_ENGINE_SHADER_DIR "/entity.vert", VOXEL_ENGINE_SHADER_DIR "/entity.frag");

        m_render_pipeline.addPass<GLWorldRenderPass>(
            *(m_shader_manager.get("world")),
            m_world
        );

        m_render_pipeline.addPass<GLUIRenderPass>(
            *(m_shader_manager.get("ui")));

        m_render_pipeline.addPass<GLEntityRenderPass>(*(m_shader_manager.get("entity")), m_world);
    }

    UserProfile loadProfile() {
        const std::string& path = m_config.profile_file_path;
        std::ifstream file(path);

        if (!file) {
            UserProfile profile{};
            profile.id = voxeng::math::generate_seed();
            return profile;
        }

        UserProfile profile{};
        std::string uuid_s;
        if (!std::getline(file, uuid_s)) {
            std::cerr << "<voxeng> error: Invalid or empty profile file at " << path << "\n";
            profile.id = voxeng::math::generate_seed();
            return profile;
        }
        profile.id = std::stoull(uuid_s);

        if (!std::getline(file, profile.username)) {
            std::cerr << "<voxeng> error: Missing username in profile file at " << path << "\n";
        }

        return profile;
    }

    void saveProfile() {
        const std::string& path = m_config.profile_file_path;
        std::ofstream file(path);

        if (!file) {
            std::cerr << "<voxeng> error: Failed to open/create profile file at " << path << "\n";
            return;
        }

        file << m_user.id << "\n" << m_user.username;
    }


    bool playerSetVoxel(uint8_t max_reach, VoxelID new_voxel) {
        if (!m_world.has_value() || !m_camera.has_value()) {
            return false;
        }

        Vec3f player_pos = m_camera->getPos();
        Vec3f camera_dir = m_camera->getFront();
        Vec3f ray_cast = player_pos;
        for (int i = 0; i < max_reach; ++i) {
            ray_cast += camera_dir;
            const VoxelType& v = m_world->getVoxel(ray_cast);
            if (v.getId() != 0) {
                Vec3f to_place = new_voxel > 0 ? ray_cast - camera_dir : ray_cast;
                m_connection.pushRequest(network::PlayerVoxelRequest{
                    m_user.id,
                    to_place,
                    new_voxel
                });
                return true;
            }
        }
        return false;
    }


    ~Impl() = default;
};


Client::Client(ClientConfig& config, network::IClientConnection& connection): m_impl(std::make_unique<Impl>(config, connection))  {}
Client::~Client() = default;

const std::string& Client::getUsername() const {
    return m_impl->m_user.username;
}

void Client::setUsername(std::string username) {
    m_impl->m_user.username = std::move(username);
    m_impl->saveProfile();
}

UserID Client::getUserId() const {
    return m_impl->m_user.id;
}

uint8_t Client::getRenderDistance() const {
    return m_impl->m_render_distance;
}

WorldCoord Client::getPlayerPos() const {
    if (!m_impl->m_camera.has_value()) {
        return {0.0f, 0.0f, 0.0f};
    }
    return m_impl->m_camera->getPos();
}


void Client::setRenderDistance(uint8_t render_distance) {
    m_impl->m_render_distance = render_distance;
}

void Client::moveCamera(float xoffset, float yoffset, bool constrain_pitch) {
    if (!m_impl->m_camera.has_value()) {
        return;
    }
    m_impl->m_camera->processEulerMovement(xoffset, yoffset, constrain_pitch);
}

void Client::zoomCamera(float yoffset) {
    if (!m_impl->m_camera.has_value()) {
        return;
    }
    m_impl->m_camera->processZoom(yoffset);
}


void Client::setPlayerSpeed(float speed) {
    m_impl->m_camera->setMovementSpeed(speed);
}

float Client::getPlayerSpeed() const {
    return m_impl->m_camera->getMovementSpeed();
}

void Client::movePlayer(Movement mov, float delta_time) {
    if (!m_impl->m_camera.has_value()) {
        return;
    }
    m_impl->m_camera->processMovement(mov, delta_time);
    m_impl->m_position_dirty = true;
}


void Client::placeVoxel(uint8_t max_reach, VoxelID voxel) {
    m_impl->playerSetVoxel(max_reach, voxel);
}
void Client::removeVoxel(uint8_t max_reach) {
    m_impl->playerSetVoxel(max_reach, 0); // 0 since AIR = 0 as convention in this codebase
}

void Client::sendChatMessage(const std::string& content) {
    if (content.empty() || !m_impl->m_world.has_value()) {
        return;
    }

    m_impl->m_connection.pushRequest(network::SendChatRequest{
        m_impl->m_user,
        content
    });
}

std::vector<Message> Client::getAllMessages() const {
    return m_impl->m_chat_history.getAllMessages();
}

std::vector<Message> Client::getRecentMessages(size_t count) const {
    if (count > m_impl->m_chat_history.size()) {
        return m_impl->m_chat_history.getAllMessages();
    }
    return m_impl->m_chat_history.getNLastMessages(count);
}

void Client::createWorld(const std::string& name) {
    m_impl->m_connection.pushRequest(network::CreateWorldRequest{name});
}

void Client::loadWorld(const std::string& world_path) {
    m_impl->m_connection.pushRequest(network::LoadWorldRequest{world_path});
    joinWorld();
}

void Client::saveWorld() {
    m_impl->m_connection.pushRequest(network::SaveWorldRequest{});
}

void Client::leaveWorld() {
    m_impl->m_world.reset();
    m_impl->m_camera.reset();
    m_impl->m_position_dirty = false;
}

void Client::joinWorld() {
    m_impl->m_connection.pushRequest(network::JoinWorldRequest{
        m_impl->m_user,
        m_impl->m_render_distance
    });
}

const std::vector<std::string>& Client::listWorlds() const {
    //TODO: maybe handle the state where the request is sent but the cache is not 
    //filled yet, in this case we should make the gui layer aware of that (by having a isWorldListLoaded method or something) and he should render maybe a spinner instead of No world found
    if (!m_impl->m_worlds_cache_loaded) {
        network::ListWorldsRequest req;
        m_impl->m_connection.pushRequest(req);
    }

    return m_impl->m_worlds_paths_cache;
}

bool Client::isWorldOpen() const {
    return m_impl->m_world.has_value();
}

const std::vector<VoxelType> *Client::getWorldVoxelTypes() const {
    if (!m_impl->m_world.has_value()) {
        return nullptr;
    }

    return &m_impl->m_world->getVoxelTypes();
}

const VoxelType *Client::getWorldVoxelType(VoxelID vid) const {
    const auto *types = getWorldVoxelTypes();
    if (vid >= types->size()) {
        std::cerr << "<voxeng> WARNING: Voxel Type " << vid << " does not exist.\n";
        return nullptr;
    }
    return &(*types)[vid];
}

void Client::handleEvent(network::WorldLoadEvent& event) {
    m_impl->m_camera.emplace(event.spawn_pos, m_impl->m_config.width, m_impl->m_config.height);

    m_impl->m_world.reset();
    m_impl->m_world.emplace(event.voxel_types, m_impl->m_render_distance);

    auto *world_pass = m_impl->m_render_pipeline.getPass<GLWorldRenderPass>();
    if (world_pass) {
        world_pass->loadTextures(m_impl->m_world->getVoxelTypes());
    }
}

void Client::handleEvent(network::ChunkDataEvent& event) {
    if (!m_impl->m_world.has_value()) return;
    m_impl->m_world->loadChunk(event.id, std::move(event.data));
}

void Client::handleEvent(const network::ListWorldsEvent& event) {
    m_impl->m_worlds_paths_cache = event.paths;
}

void Client::handleEvent(const network::ChunkUnloadEvent& event) {
    if (m_impl->m_world.has_value()) {
        m_impl->m_world->unloadChunk(event.id);
    }
}

void Client::handleEvent(const network::NewWorldCreatedEvent& event) {
    m_impl->m_worlds_paths_cache.push_back(event.path);
}

void Client::handleEvent(const network::ServerErrorEvent& event) {
    m_impl->m_chat_history.pushMessage(Message{{0, "error"}, event.kind + ": " + event.message, event.timestamp});
}

void Client::handleEvent(const network::VoxelChangedEvent& event) {
    if (m_impl->m_world.has_value()) {
        m_impl->m_world->setVoxel(event.chunk_id, event.chunk_pos, event.new_voxel);
    }
}

void Client::handleEvent(const network::ChatMessageEvent& event) {
    m_impl->m_chat_history.pushMessage(Message{event.message});
}

void Client::handleEvent(const network::ChatHistoryEvent& event) {
    for (const auto& msg : event.messages) {
        m_impl->m_chat_history.pushMessage(Message{msg});
    }
}


void Client::handleEvent(const network::EntitySpawnEvent& event) {
    if (!m_impl->m_world.has_value()) {
        return;
    }

    //TODO: maybe here add rotation
    m_impl->m_world->spawnEntity(event.entity_id, RenderEntity{
        event.model_name,
        event.position
    });
}

void Client::handleEvent(const network::EntityUpdateEvent& event) {
    if (!m_impl->m_world.has_value()) {
        return;
    }

    //TODO: handle different kind of update events

    RenderEntity *ent = m_impl->m_world->getEntity(event.entity_id);
    ent->position = event.position;
}

void Client::handleEvent(const network::EntityDespawnEvent& event) {
    if (!m_impl->m_world.has_value()) {
        return;
    }

    m_impl->m_world->despawnEntity(event.entity_id);
}


void Client::render() {
    if (!m_impl->m_camera.has_value()) {
        m_impl->m_render_pipeline.render();
        return;
    }
    m_impl->m_render_pipeline.render(m_impl->m_camera.value());
}

void Client::update() {
    auto current_time = std::chrono::steady_clock::now();
    if (current_time - m_impl->m_previous_tick < TICK_DURATION) {
        return;
    }
    m_impl->m_previous_tick = current_time;

    if (m_impl->m_position_dirty && m_impl->m_camera.has_value()) {
        m_impl->m_connection.pushRequest(network::PlayerPositionRequest {
            m_impl->m_user.id,
            m_impl->m_camera->getPos()
        });
        m_impl->m_position_dirty = false;
    }

    // Event handling
    while (auto evt = m_impl->m_connection.pollEvent()) {
        std::visit([this](auto&& e) { handleEvent(e); }, *evt);
    }

}

}
