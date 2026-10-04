#include "glworld_render_pass.h"
#include "rendering/opengl/gl_mesh.h"
#include "rendering/opengl_texture.h"
#include "rendering/world_mesh_builder.h"
#include "voxel_engine/voxel_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <vector>

namespace voxeng::client {

// CPU time spent rebuilding chunk meshes per frame (at least one mesh is always built)
static constexpr auto MESH_BUDGET_PER_FRAME = std::chrono::microseconds(2000);

static glm::vec3 toGlm(Vec3f v) {
    return {v.x, v.y, v.z};
}

static bool chunkInFrustum(const Frustum& frustum, const Chunk& chunk) {
    glm::vec3 min = toGlm(chunk.getWorldPos());
    glm::vec3 max = min + glm::vec3(Chunk::WIDTH, Chunk::HEIGHT, Chunk::DEPTH);
    return frustum.intersectsAABB(min, max);
}

GLWorldRenderPass::GLWorldRenderPass(const Shader& shader,
				     const std::optional<ClientWorld>& world)
    : m_shader_prog(shader), m_world(world) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable blending for transparent blocks (like water)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(77.0f/255.0f, 109.0f/255.0f, 157.0f/255.0f, 1.0f);
}

void GLWorldRenderPass::loadTextures(const std::vector<VoxelType>& voxel_types) {
    m_textures.loadTextures(voxel_types);
    m_textures.bind();
    m_world_mesh_builder.setVoxelTypes(voxel_types);
}

void GLWorldRenderPass::buildDirtyMeshes(const Camera& camera, const Frustum& frustum) {
    struct DirtyChunk {
        ChunkID id;
        const Chunk* chunk;
        bool visible;
        float dist2;
    };

    std::vector<DirtyChunk> dirty;
    glm::vec3 cam_pos = toGlm(camera.getPos());
    glm::vec3 half_chunk = glm::vec3(Chunk::WIDTH, Chunk::HEIGHT, Chunk::DEPTH) * 0.5f;
    for (auto& [chunk_id, c]: m_world->getChunks()) {
        if (!c.isRenderDirty()) continue;
        glm::vec3 d = toGlm(c.getWorldPos()) + half_chunk - cam_pos;
        dirty.push_back({chunk_id, &c, chunkInFrustum(frustum, c), glm::dot(d, d)});
    }

    m_stats.chunks_meshed = 0;
    m_stats.mesh_queue_size = dirty.size();
    if (dirty.empty()) return;

    // visible chunks first, then nearest first
    std::sort(dirty.begin(), dirty.end(), [](const DirtyChunk& a, const DirtyChunk& b) {
        if (a.visible != b.visible) return a.visible;
        return a.dist2 < b.dist2;
    });

    auto start = std::chrono::steady_clock::now();
    for (const auto& d: dirty) {
        auto& mesh = m_chunk_meshes[d.id];
        if (!mesh) {
            mesh = std::make_shared<GLMesh>();
        }
        mesh->upload(m_world_mesh_builder.buildMesh(*d.chunk));
        d.chunk->clearRenderDirty();
        m_stats.chunks_meshed++;

        if (std::chrono::steady_clock::now() - start >= MESH_BUDGET_PER_FRAME) {
            break;
        }
    }
    m_stats.mesh_queue_size -= m_stats.chunks_meshed;
}

void GLWorldRenderPass::render(const Camera& camera) {
    if (!m_world.has_value()) return;

    glEnable(GL_DEPTH_TEST);
    m_shader_prog.bind();

    // MVP matrices calc
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix();

    // set shader MVP uniforms
    m_shader_prog.setUniformMatrix4fv("model", glm::value_ptr(model));
    m_shader_prog.setUniformMatrix4fv("view", glm::value_ptr(view));
    m_shader_prog.setUniformMatrix4fv("projection", glm::value_ptr(projection));

    // voxel texture TODO: change approach
    m_shader_prog.setUniform1i("uTextures", 0);

    Frustum frustum = Frustum::fromMatrix(projection * view);
    buildDirtyMeshes(camera, frustum);

    m_stats.chunks_loaded = m_world->getChunks().size();
    m_stats.chunks_drawn = 0;
    for (auto& [chunk_id, c]: m_world->getChunks()) {
        auto it = m_chunk_meshes.find(chunk_id);
        // not meshed yet
        if (it == m_chunk_meshes.end()) continue;
        if (!chunkInFrustum(frustum, c)) continue;
        it->second->draw();
        m_stats.chunks_drawn++;
    }

    // Remove stale mesh entries for unloaded chunks (once per second)
    auto now = std::chrono::steady_clock::now();
    if (now - m_last_cleanup >= std::chrono::seconds(1)) {
        m_last_cleanup = now;
        const auto& active_chunks = m_world->getChunks();
        for (auto it = m_chunk_meshes.begin(); it != m_chunk_meshes.end(); ) {
            if (active_chunks.find(it->first) == active_chunks.end()) {
                it = m_chunk_meshes.erase(it);
            } else {
                ++it;
            }
        }
    }

    m_shader_prog.unbind();
}

}
