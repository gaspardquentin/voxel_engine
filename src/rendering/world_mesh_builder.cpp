#include "world_mesh_builder.h"
#include "voxel_engine/voxel_types.h"
#include <cstdint>
#include <iostream>


Vec3i direction(VoxelFace vf) {
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

void WorldMeshBuilder::emitFace(MeshData& mesh_data, const Chunk& chunk, ChunkCoord voxel_pos, VoxelFace face, const VoxelType& voxel) const {
    // Before pushing, record the starting index
    uint32_t startIndex = static_cast<uint32_t>(mesh_data.vertices.size());

    // Push the 4 vertices for this face
    for (const Vertex& v: VOXEL_FACE_VERTICES[static_cast<uint8_t>(face)]) {
        mesh_data.vertices.push_back({
            Vec3f{voxel_pos} + chunk.getWorldPos() + v.pos,
            v.normal,
            {v.uv.x, v.uv.y, v.uv.z + voxel.getId()}
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
    MeshData mesh_data;
    for (unsigned int x = 0; x < Chunk::WIDTH; ++x) {
	for (unsigned int y = 0; y < Chunk::HEIGHT; ++y) {
	    for (unsigned int z = 0; z < Chunk::DEPTH; ++z) {
		const VoxelType& voxel = chunk.getVoxel({ x, y, z });

		if (voxel.getId() == 0 || voxel.getName() == "air") {
		    continue;
		}

                for (uint8_t vfi = 0; vfi <= static_cast<uint8_t>(VoxelFace::BOTTOM); vfi++) {
                    VoxelFace face = static_cast<VoxelFace>(vfi);
                    // Use signed int for neighbor calculation to handle negative offsets
                    Vec3i neighborPos = Vec3i{static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)} + direction(face);
                    VoxelType neighbor = chunk.getVoxelType(0);
                    // Explicitly cast to ChunkCoord (Vec3u) for boundary check
                    // If neighborPos has negative values, casting to unsigned makes them huge, failing the check correctly.
                    ChunkCoord neighborCoord = ChunkCoord(neighborPos);
                    if (chunk.positionInChunk(neighborCoord)) {
                        neighbor = chunk.getVoxel(neighborCoord);
                    }

                    if (neighbor.isTransparent()) {
                        emitFace(mesh_data, chunk, {x, y, z}, face, voxel);
                    }
                }
	    }
	}
    }
    return mesh_data;
}
