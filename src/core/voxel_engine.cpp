#include "voxel_engine/voxel_engine.h"
#include "voxel_engine/save_manager.h"
#include "voxel_engine/world.h"
#include "voxel_engine/math_utils.h"
#include "voxel_engine/camera.h"
#include "voxel_engine/chat.h"
#include "voxel_engine/command_registry.h"
#include "rendering/render_pipeline.h"
#include "rendering/shader_manager.h"
#include "rendering/passes/glui_render_pass.h"
#include "rendering/passes/gl_entity_render_pass.h"
#include <chrono>
#include <iostream>

const int TICK_RATE = 20;
static constexpr auto TICK_DURATION = std::chrono::milliseconds((1000/TICK_RATE));

class VoxelEngine::Impl {
public:
    World m_world{generate_seed()};
    Camera m_camera;
    ShaderManager m_shader_manager;
    RenderPipeline m_render_pipeline;
    voxeng::SaveManager m_save_manager;
    voxeng::Chat m_chat{256};
    voxeng::CommandRegistry m_command_registry;
    std::chrono::time_point<std::chrono::steady_clock> m_previous_tick = std::chrono::steady_clock::now();
    Vec3f m_last_generation_pos = {0.0f, 0.0f, 0.0f};

    Impl(const VoxelEngineConfig& config):
        m_camera({0.0f, 0.0f, 3.0f}, config.width, config.height)
    {
        m_world.setSaveManager(&m_save_manager);

        for (const auto& cmd : voxeng::DEFAULT_COMMANDS) {
            m_command_registry.registerCommand(cmd.first, cmd.second);
        }

        m_shader_manager.load("world", config.world_vertex_shader, config.world_fragment_shader);
        m_shader_manager.load("ui", config.ui_vertex_shader, config.ui_fragment_shader);
        m_shader_manager.load("entity", VOXEL_ENGINE_SHADER_DIR "/simple_mvp.vert", VOXEL_ENGINE_SHADER_DIR "/simple.frag");

        m_render_pipeline.addPass<GLWorldRenderPass>(
            *(m_shader_manager.get("world")),
            m_world
        );
        m_render_pipeline.addPass<GLUIRenderPass>(
            *(m_shader_manager.get("ui")));

        m_render_pipeline.addPass<GLEntityRenderPass>(*(m_shader_manager.get("entity")));

        auto *world_pass = m_render_pipeline.getPass<GLWorldRenderPass>();
        if (world_pass) {
            world_pass->loadTextures();
        }
    }

    ~Impl() {
        m_world.flushAllDirtyChunks();
        m_save_manager.closeWorld();
    }
};

VoxelEngine::VoxelEngine(const VoxelEngineConfig& config):
    m_impl(new VoxelEngine::Impl(config)) {}

VoxelEngine::~VoxelEngine() { delete m_impl; }

World& VoxelEngine::getWorld() {
    return m_impl->m_world;
}

void VoxelEngine::setWorld(World&& world) {
    m_impl->m_world = std::move(world);
    m_impl->m_world.setSaveManager(&m_impl->m_save_manager);
}

Camera& VoxelEngine::getCamera() {
    return m_impl->m_camera;
}

const Camera& VoxelEngine::getCamera() const {
    return m_impl->m_camera;
}

voxeng::SaveManager& VoxelEngine::getSaveManager() {
    return m_impl->m_save_manager;
}

voxeng::Chat& VoxelEngine::getChat() {
    return m_impl->m_chat;
}

voxeng::CommandRegistry& VoxelEngine::getCommandRegistry() {
    return m_impl->m_command_registry;
}

void VoxelEngine::handleChatInput(const std::string& sender_id, const std::string& content) {
    if (content.empty()) {
        return;
    }

    if (content[0] == '/') {
        voxeng::CommandContext ctx { m_impl->m_chat, m_impl->m_world };
        bool ok = m_impl->m_command_registry.tryExecute(content, ctx);
        if (!ok) {
            m_impl->m_chat.sendMessage("error", "Invalid command.");
        }
        return;
    }

    m_impl->m_chat.sendMessage(sender_id, content);
}

void VoxelEngine::createNewWorld(const std::string& name) {
    uint64_t seed = generate_seed();
    if (!m_impl->m_save_manager.createWorld(name, seed)) {
        std::cerr << "<voxeng> Failed to create world: " << name << "\n";
        return;
    }
    World new_world{seed};
    new_world.setSaveManager(&m_impl->m_save_manager);
    setWorld(std::move(new_world));
}

void VoxelEngine::loadExistingWorld(const std::string& world_path) {
    if (!m_impl->m_save_manager.openWorld(world_path)) {
        std::cerr << "<voxeng> Failed to open world: " << world_path << "\n";
        return;
    }
    uint64_t seed = m_impl->m_save_manager.getMetadata().world_seed;
    World new_world{seed, false};
    new_world.setSaveManager(&m_impl->m_save_manager);
    setWorld(std::move(new_world));
    m_impl->m_world.updateChunks({0.0f, 0.0f, 0.0f});
}

void VoxelEngine::render() {
    m_impl->m_render_pipeline.render(m_impl->m_camera);
}

void VoxelEngine::update(float delta_time) {
    auto current_time = std::chrono::steady_clock::now();
    if (current_time - m_impl->m_previous_tick < TICK_DURATION) {
        return;
    }

    //TODO: be sure the steady clock fix was not a scam


    m_impl->m_previous_tick = current_time;
    m_impl->m_world.update();

    const Vec3f& player_pos = m_impl->m_camera.getPos();
    constexpr float GENERATION_TRESHOLD = 20.0f;
    if (Vec3f::dist(m_impl->m_last_generation_pos, player_pos) > GENERATION_TRESHOLD) {
        m_impl->m_world.updateChunks(player_pos);
        m_impl->m_last_generation_pos = player_pos;
    }
}

void VoxelEngine::processMovementPlayer(Movement mov, float delta_time) {
    m_impl->m_camera.processMovement(mov, delta_time);
}

void VoxelEngine::processMovementCamera(float xoffset, float yoffset, bool constrain_pitch) {
    m_impl->m_camera.processEulerMovement(xoffset, yoffset, constrain_pitch);
}
void VoxelEngine::processZoomCamera(float yoffset) {
    m_impl->m_camera.processZoom(yoffset);
}

bool VoxelEngine::playerPlaceVoxel(uint8_t max_reach, VoxelID voxel) {
    if (voxel == 0)
        return false;
    return playerSetVoxel(max_reach, voxel);
}

bool VoxelEngine::playerRemoveVoxel(uint8_t max_reach) {
    return playerSetVoxel(max_reach, 0);
}

bool VoxelEngine::playerSetVoxel(uint8_t max_reach, VoxelID new_voxel) {
    Vec3f player_pos = m_impl->m_camera.getPos();
    Vec3f camera_dir = m_impl->m_camera.getFront();
    Vec3f ray_cast = player_pos;
    for (int i = 0; i < max_reach; ++i) {
        ray_cast += camera_dir;
        const VoxelType& v = m_impl->m_world.getVoxel(ray_cast);
        if (v.getId() != 0) {
            Vec3f to_place = new_voxel > 0 ? ray_cast - camera_dir : ray_cast;
            m_impl->m_world.setVoxel(to_place, new_voxel);
            return true;
        }
    }
    return false;
}
