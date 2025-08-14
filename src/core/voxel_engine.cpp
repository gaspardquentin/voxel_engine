#include "voxel_engine/voxel_engine.h"
#include "voxel_engine/world.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"

class VoxelEngine::Impl {
public:
    World m_world;
    Camera m_camera;
    Renderer m_renderer;

    Impl(): m_renderer(
        VOXEL_ENGINE_SHADER_DIR  "/vertex.vert",
        VOXEL_ENGINE_SHADER_DIR  "/fragment.frag") {}
    ~Impl() {}
};

VoxelEngine::VoxelEngine(): m_impl(new VoxelEngine::Impl()) {}
VoxelEngine::~VoxelEngine() { delete m_impl; }

World& VoxelEngine::getWorld() {
    return m_impl->m_world;
}

void VoxelEngine::render() {
    m_impl->m_renderer.render(m_impl->m_world, m_impl->m_camera);
}
