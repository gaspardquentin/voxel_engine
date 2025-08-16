#include "voxel_engine/voxel_engine.h"
#include "voxel_engine/world.h"
#include "voxel_engine/math_utils.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"

class VoxelEngine::Impl {
public:
    World m_world;
    Camera m_camera;
    Renderer m_renderer;

    Impl(unsigned int screen_width, unsigned int screen_height):
        m_camera({0.0f, 0.0f, 3.0f}),//TODO: change this
        m_renderer(
            VOXEL_ENGINE_SHADER_DIR  "/vertex.vert",
            VOXEL_ENGINE_SHADER_DIR  "/fragment.frag",
            screen_width,
            screen_height
        )
    {}
    ~Impl() {
        m_renderer.destroy();
    }
};

VoxelEngine::VoxelEngine(unsigned int screen_width, unsigned int screen_height):
    m_impl(new VoxelEngine::Impl(screen_width, screen_height)) {}
VoxelEngine::~VoxelEngine() { delete m_impl; }

World& VoxelEngine::getWorld() {
    return m_impl->m_world;
}

void VoxelEngine::render() {
    m_impl->m_renderer.render(m_impl->m_world, m_impl->m_camera);
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
