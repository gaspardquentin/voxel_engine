#include "voxel_engine/server/server.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/message.h"
#include "voxel_engine/network/client_event.h"
#include "voxel_engine/network/server_request.h"
#include "voxel_engine/save_format.h"
#include "voxel_engine/server/chat.h"
#include "voxel_engine/server/commands.h"
#include "voxel_engine/server/network/local_server_connection.h"
#include "voxel_engine/server/save_manager.h"
#include "voxel_engine/server/world.h"
#include "voxel_engine/types.h"
#include "voxel_engine/user.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/world_coords.h"
#include "voxel_engine/server/entity_systems.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <variant>


namespace voxeng::server {

const int TICK_RATE = 20;
static constexpr auto TICK_DURATION = std::chrono::milliseconds((1000/TICK_RATE));

class Server::Impl {
public:
    network::IServerConnection& m_connection;
    SaveManager m_save_manager;
    Chat m_chat{256};
    CommandRegistry m_command_registry;
    std::unique_ptr<World> m_world; //TODO: maybe add multiple worlds support (and portals) or remove world loading logic and set a unique world per server (and the server lifetime linked with the world's one)
    std::unordered_map<UserID, UserProfile> m_connected_users;
    std::chrono::time_point<std::chrono::steady_clock> m_previous_tick = std::chrono::steady_clock::now();

    Impl(network::IServerConnection& connection):
        m_connection(connection) {
        registerDefaultCommands(m_command_registry);
    }

    ~Impl() = default;
};

Server::Server(network::IServerConnection& connection):
    m_impl(std::make_unique<Impl>(connection)) {}

Server::~Server() {
    if (isWorldLoaded()) {
        saveAndCloseWorld();
    }
}

void Server::update(float delta_time) {
    auto current_time = std::chrono::steady_clock::now();
    if (current_time - m_impl->m_previous_tick < TICK_DURATION) {
        return;
    }
    m_impl->m_previous_tick = current_time;

    // Only process the latest position request to avoid redundant updateChunks calls
    std::optional<network::PlayerPositionRequest> latest_position;
    while (auto req = m_impl->m_connection.pollRequest()) {
        std::visit([this, &latest_position](auto&& r) {
            using T = std::decay_t<decltype(r)>;
            if constexpr (std::is_same_v<T, network::PlayerPositionRequest>) {
                latest_position = r;
            } else {
                handleRequest(r);
            }
        }, *req);
    }
    if (latest_position) {
        handleRequest(*latest_position);
    }

    if (m_impl->m_world) {
        m_impl->m_world->update();

        auto& reg = m_impl->m_world->getRegistry();
        movement_system::update(reg, delta_time);
        //TODO: TODO: maybe separate into a sync method or something
        auto view = m_impl->m_world->getRegistry().view<Position>();
        for (auto [ent, pos]: view->each()) {
            m_impl->m_connection.pushEvent(network::EntityUpdateEvent{
                static_cast<EntityID>(ent),
                pos.pos
            });
        }
    }
}

void Server::handleRequest(const network::PlayerVoxelRequest& req) {
    //TODO: once User permissions are implemented, forbid/permit voxel change based on user id
    m_impl->m_world->setVoxel(req.position, req.voxel);
}

void Server::handleRequest(const network::PlayerPositionRequest& req) {
    if (!isWorldLoaded()) return;
    m_impl->m_world->updateChunks(req.position);
}

void Server::handleRequest(const network::CreateWorldRequest& req) {
    WorldConfig config;
    if (!m_impl->m_save_manager.createWorld(req.name, config.seed, config.voxel_types)) {
        network::ServerErrorEvent err{"CreateWorldError", "Failed to create world: " + req.name};
        m_impl->m_connection.pushEvent(err);
        return;
    }

    std::string path = std::string(DEFAULT_WORLDS_DIR) + req.name;
    network::NewWorldCreatedEvent evt{path};
    m_impl->m_connection.pushEvent(evt);

    m_impl->m_save_manager.closeWorld();
}

void Server::handleRequest(const network::LoadWorldRequest& req) {
    if (isWorldLoaded()) {
        saveAndCloseWorld();
    }

    if (!m_impl->m_save_manager.openWorld(req.world_path)) {
        std::cerr << "<voxeng> Failed to open world: " << req.world_path << "\n";
        return;
    }

    const WorldMetadata& metadata = m_impl->m_save_manager.getMetadata();
    m_impl->m_world = std::make_unique<World>(
        m_impl->m_connection,
        metadata.voxel_types,
        metadata.world_seed,
        false
    );
    //m_impl->m_world.updateChunks({0.0f, 0.0f, 0.0f});
    m_impl->m_world->setSaveManager(&m_impl->m_save_manager);
}

void Server::handleRequest(const network::SaveWorldRequest&) {
    if (!isWorldLoaded()) return;
    m_impl->m_world->flushAllDirtyChunks();
}

void Server::handleRequest(const network::JoinWorldRequest& req) {
    if (!isWorldLoaded()) {
        network::ServerErrorEvent err {"JoinError", "Cannot join world : no world loaded on the server."};
        m_impl->m_connection.pushEvent(err);
        std::cerr << "<voxeng::server> join error: user " << req.user.id << " tried to join but no world loaded.\n";
        return;
    }

    // Register connected user
    m_impl->m_connected_users[req.user.id] = req.user;

    network::WorldLoadEvent world_evt{
        m_impl->m_world->getVoxelTypes()
    };
    m_impl->m_connection.pushEvent(world_evt);

    // Send chat history to newly joined player
    auto messages = m_impl->m_chat.getAllMessages();
    if (!messages.empty()) {
        m_impl->m_connection.pushEvent(network::ChatHistoryEvent{std::move(messages)});
    }

    m_impl->m_world->updateChunks(world_evt.spawn_pos);

}

void Server::handleRequest(const network::ListWorldsRequest&) {
    network::ListWorldsEvent evt{};
    for (const std::string& path : m_impl->m_save_manager.listWorlds()) {
        evt.paths.push_back(path);
    }
    m_impl->m_connection.pushEvent(evt);
}


void Server::handleRequest(const network::SendChatRequest& req) {
    if (!isWorldLoaded() || req.content.empty()) {
        return;
    }

    //TODO: maybe here add a ban word list (or in Chat.cpp)

    if (req.content[0] == '/') {
        CommandContext ctx {
            *m_impl->m_world,
            req.user.id,
            [this](const std::string& message) {
                m_impl->m_connection.pushEvent(network::ServerErrorEvent{"CommandError", message});
            },
            [this, req](const std::string& message) {
                m_impl->m_connection.pushEvent(network::ChatMessageEvent{
                    {{0, req.content}, message, std::chrono::system_clock::now()}
                });
            }
        };
        bool ok = m_impl->m_command_registry.tryExecute(req.content, ctx);
        if (!ok) {
            m_impl->m_connection.pushEvent(network::ServerErrorEvent{"CommandError", "Unknown command."});
        }
        return;
    }

    m_impl->m_chat.sendMessage(req.user, req.content);

    // Broadcast the new message to all clients
    m_impl->m_connection.pushEvent(network::ChatMessageEvent{
        {req.user, req.content, std::chrono::system_clock::now()}
    });
}

void Server::saveAndCloseWorld() {
    if (!isWorldLoaded()) return;

    // Drain any pending requests (e.g. unprocessed voxel changes) before saving
    while (auto req = m_impl->m_connection.pollRequest()) {
        std::visit([this](auto&& r) { handleRequest(r); }, *req);
    }

    m_impl->m_world->flushAllDirtyChunks();

    WorldMetadata meta = m_impl->m_save_manager.getMetadata();
    // Update known_players with currently connected users
    for (const auto& pair : m_impl->m_connected_users) {
        UserID uid = pair.first;
        auto it = std::find_if(meta.known_players.begin(), meta.known_players.end(),
            [uid](const PlayerSaveData& p) { return p.uid == uid; });
        if (it == meta.known_players.end()) {
            meta.known_players.push_back({uid, {}});
        }
    }
    m_impl->m_save_manager.updateMetadata(meta);
    m_impl->m_save_manager.closeWorld();
    m_impl->m_world.reset();
    m_impl->m_connected_users.clear();
}

bool Server::isWorldLoaded() const {
    return m_impl->m_world != nullptr;
}

}
