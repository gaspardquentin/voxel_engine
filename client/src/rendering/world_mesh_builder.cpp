#include "world_mesh_builder.h"
#include "voxel_engine/voxel_types.h"
#include <cstdint>
#include <iostream>

namespace voxeng::client {

static Vec3i direction(VoxelFace vf) {
    switch (vf) {
        case VoxelFace::FRONT: return {0, 0, 1};
        case VoxelFace::BACK: return {0, 0, -1};
        case VoxelFace::LEFT: return {-1, 0, 0};
        case VoxelFace::RIGHT: return {1, 0, 0};
        case VoxelFace::TOP: return {0, 1, 0};
        case VoxelFace::BOTTOM: return {0, -1, 0};
    }
    std::cerr << "<voxeng> should not be here." << std::endl;
    return {0, 0, 0};
}

const std::array<std::array<Vertex, 4>, 6> VOXEL_FACE_VERTICES = {
    // FRONT face (z+)
    std::array<Vertex, 4>{
        Vertex{{0, 0, 1}, {0, 0, 1}, {0, 0, 0}},
        Vertex{{1, 0, 1}, {0, 0, 1}, {1, 0, 0}},
        Vertex{{1, 1, 1}, {0, 0, 1}, {1, 1, 0}},
        Vertex{{0, 1, 1}, {0, 0, 1}, {0, 1, 0}}
    },
    // BACK face (z-)
    std::array<Vertex, 4>{
        Vertex{{1, 0, 0}, {0, 0, -1}, {0, 0, 0}},
        Vertex{{0, 0, 0}, {0, 0, -1}, {1, 0, 0}},
        Vertex{{0, 1, 0}, {0, 0, -1}, {1, 1, 0}},
        Vertex{{1, 1, 0}, {0, 0, -1}, {0, 1, 0}}
    },
    // LEFT face (x-)
    std::array<Vertex, 4>{
        Vertex{{0, 0, 0}, {-1, 0, 0}, {0, 0, 0}},
        Vertex{{0, 0, 1}, {-1, 0, 0}, {1, 0, 0}},
        Vertex{{0, 1, 1}, {-1, 0, 0}, {1, 1, 0}},
        Vertex{{0, 1, 0}, {-1, 0, 0}, {0, 1, 0}}
    },
    // RIGHT face (x+)
    std::array<Vertex, 4>{
        Vertex{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}},
        Vertex{{1, 0, 0}, {1, 0, 0}, {1, 0, 0}},
        Vertex{{1, 1, 0}, {1, 0, 0}, {1, 1, 0}},
        Vertex{{1, 1, 1}, {1, 0, 0}, {0, 1, 0}}
    },
    // TOP face (y+)
    std::array<Vertex, 4>{
        Vertex{{0, 1, 1}, {0, 1, 0}, {0, 0, 0}},
        Vertex{{1, 1, 1}, {0, 1, 0}, {1, 0, 0}},
        Vertex{{1, 1, 0}, {0, 1, 0}, {1, 1, 0}},
        Vertex{{0, 1, 0}, {0, 1, 0}, {0, 1, 0}}
    },
    // BOTTOM face (y-)
    std::array<Vertex, 4>{
        Vertex{{0, 0, 0}, {0, -1, 0}, {0, 0, 0}},
        Vertex{{1, 0, 0}, {0, -1, 0}, {1, 0, 0}},
        Vertex{{1, 0, 1}, {0, -1, 0}, {1, 1, 0}},
        Vertex{{0, 0, 1}, {0, -1, 0}, {0, 1, 0}}
    }
};

WorldMeshBuilder::WorldMeshBuilder() {
    m_is_air.fill(1);
    m_transparent.fill(1);
}

void WorldMeshBuilder::setVoxelTypes(const std::vector<VoxelType>& voxel_types) {
    // unknown ids are treated as air, like Chunk::getVoxel does
    m_is_air.fill(1);
    m_transparent.fill(1);
    for (size_t i = 0; i < voxel_types.size() && i < m_is_air.size(); i++) {
        m_is_air[i] = (i == 0 || voxel_types[i].getName() == "air");
        m_transparent[i] = voxel_types[i].isTransparent();
    }
}

void WorldMeshBuilder::emitFace(MeshData& mesh_data, Vec3f voxel_world_pos, VoxelFace face, VoxelID voxel) const {
    // Before pushing, record the starting index
    uint32_t startIndex = static_cast<uint32_t>(mesh_data.vertices.size());

    // Push the 4 vertices for this face
    for (const Vertex& v: VOXEL_FACE_VERTICES[static_cast<uint8_t>(face)]) {
        mesh_data.vertices.push_back({
            voxel_world_pos + v.pos,
            v.normal,
            {v.uv.x, v.uv.y, v.uv.z + voxel}
         });
    }

    // Add 6 indices (two triangles)
    mesh_data.indices.push_back(startIndex);
    mesh_data.indices.push_back(startIndex + 1);
    mesh_data.indices.push_back(startIndex + 2);
    mesh_data.indices.push_back(startIndex + 2);
    mesh_data.indices.push_back(startIndex + 3);
    mesh_data.indices.push_back(startIndex);
}

MeshData WorldMeshBuilder::buildMesh(const Chunk& chunk) const {
    return buildMesh(chunk.getRawData(), chunk.getWorldPos());
}

MeshData WorldMeshBuilder::buildMesh(const std::array<VoxelID, CHUNK_SIZE>& data, Vec3f world_pos) const {
    // rough guess of a terrain chunk surface, avoids most reallocations
    constexpr size_t RESERVED_FACES = Chunk::WIDTH * Chunk::DEPTH * 3;
    MeshData mesh_data;
    mesh_data.vertices.reserve(RESERVED_FACES * 4);
    mesh_data.indices.reserve(RESERVED_FACES * 6);

    // iterate in memory order (see Chunk::linearIndex)
    for (unsigned int y = 0; y < Chunk::HEIGHT; ++y) {
        for (unsigned int z = 0; z < Chunk::DEPTH; ++z) {
            for (unsigned int x = 0; x < Chunk::WIDTH; ++x) {
                VoxelID voxel = data[Chunk::linearIndex({x, y, z})];
                if (m_is_air[voxel]) {
                    continue;
                }
                bool voxel_transparent = m_transparent[voxel];
                Vec3f voxel_world_pos = world_pos + Vec3f{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};

                for (uint8_t vfi = 0; vfi <= static_cast<uint8_t>(VoxelFace::BOTTOM); vfi++) {
                    VoxelFace face = static_cast<VoxelFace>(vfi);
                    Vec3i dir = direction(face);
                    // negative values wrap to huge unsigned values and fail the bounds check
                    ChunkCoord n{x + dir.x, y + dir.y, z + dir.z};

                    // neighbors outside the chunk are considered air
                    VoxelID neighbor = 0;
                    if (n.x < Chunk::WIDTH && n.y < Chunk::HEIGHT && n.z < Chunk::DEPTH) {
                        neighbor = data[Chunk::linearIndex(n)];
                    }

                    // Don't emit face between two identical transparent blocks (like water next to water)
                    bool shouldEmit = m_transparent[neighbor];
                    if (voxel_transparent && voxel == neighbor) {
                        shouldEmit = false;
                    }

                    if (shouldEmit) {
                        emitFace(mesh_data, voxel_world_pos, face, voxel);
                    }
                }
            }
        }
    }
    return mesh_data;
}

} // namespace voxeng::client
