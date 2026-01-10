#include "voxel_engine/voxel_engine.h"
#include "voxel_engine/world.h"
#include "voxel_engine/math_utils.h"
#include "rendering/camera.h"
#include "rendering/render_pipeline.h"
#include "rendering/shader_manager.h"
#include "rendering/passes/glui_render_pass.h"

class VoxelEngine::Impl {
public:
    World m_world;
    Camera m_camera;
    ShaderManager m_shader_manager;
    RenderPipeline m_render_pipeline;

    Impl(const VoxelEngineConfig& config):
        m_camera({0.0f, 0.0f, 3.0f}, config.width, config.height) //TODO: change this
    {
        m_shader_manager.load("world", config.world_vertex_shader, config.world_fragment_shader);
        m_shader_manager.load("ui", config.ui_vertex_shader, config.ui_fragment_shader);

        m_render_pipeline.addPass<GLWorldRenderPass>(
            *(m_shader_manager.get("world")),
            m_world
        );
        m_render_pipeline.addPass<GLUIRenderPass>(
            *(m_shader_manager.get("ui")));

        auto *world_pass = m_render_pipeline.getPass<GLWorldRenderPass>();
        if (world_pass) {
            world_pass->loadTextures(m_world);
            world_pass->initChunkMeshes(m_world.getChunks().size());
        }
    }

    ~Impl() = default;
};

VoxelEngine::VoxelEngine(const VoxelEngineConfig& config):
    m_impl(new VoxelEngine::Impl(config)) {}

VoxelEngine::~VoxelEngine() { delete m_impl; }

World& VoxelEngine::getWorld() {
    return m_impl->m_world;
}

void VoxelEngine::render() {
    m_impl->m_render_pipeline.render(m_impl->m_camera);
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
