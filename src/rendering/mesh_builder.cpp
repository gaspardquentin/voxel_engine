#include "mesh_builder.h"
#include <iostream>
#include "voxel_engine/voxel_types.h"


Vec3i direction(VoxelFace vf) {
    switch (vf) {
        case FRONT: return {0, 0, 1};
        case BACK: return {0, 0, -1};
        case LEFT: return {-1, 0, 0};
        case RIGHT: return {1, 0, 0};
        case TOP: return {0, 1, 0};
        case BOTTOM: return {0, -1, 0};
    }
    return {0, 0, 0};
}

const std::array<std::array<Vertex, 4>, 6> VOXEL_FACE_VERTICES = {
    // FRONT face (z+)
    std::array<Vertex, 4>{
        Vertex{{0, 0, 1}, {0, 0, 1}, {0, 0}},
        Vertex{{1, 0, 1}, {0, 0, 1}, {1, 0}},
        Vertex{{1, 1, 1}, {0, 0, 1}, {1, 1}},
        Vertex{{0, 1, 1}, {0, 0, 1}, {0, 1}}
    },
    // BACK face (z-)
    std::array<Vertex, 4>{
        Vertex{{1, 0, 0}, {0, 0, -1}, {0, 0}},
        Vertex{{0, 0, 0}, {0, 0, -1}, {1, 0}},
        Vertex{{0, 1, 0}, {0, 0, -1}, {1, 1}},
        Vertex{{1, 1, 0}, {0, 0, -1}, {0, 1}}
    },
    // LEFT face (x-)
    std::array<Vertex, 4>{
        Vertex{{0, 0, 0}, {-1, 0, 0}, {0, 0}},
        Vertex{{0, 0, 1}, {-1, 0, 0}, {1, 0}},
        Vertex{{0, 1, 1}, {-1, 0, 0}, {1, 1}},
        Vertex{{0, 1, 0}, {-1, 0, 0}, {0, 1}}
    },
    // RIGHT face (x+)
    std::array<Vertex, 4>{
        Vertex{{1, 0, 1}, {1, 0, 0}, {0, 0}},
        Vertex{{1, 0, 0}, {1, 0, 0}, {1, 0}},
        Vertex{{1, 1, 0}, {1, 0, 0}, {1, 1}},
        Vertex{{1, 1, 1}, {1, 0, 0}, {0, 1}}
    },
    // TOP face (y+)
    std::array<Vertex, 4>{
        Vertex{{0, 1, 1}, {0, 1, 0}, {0, 0}},
        Vertex{{1, 1, 1}, {0, 1, 0}, {1, 0}},
        Vertex{{1, 1, 0}, {0, 1, 0}, {1, 1}},
        Vertex{{0, 1, 0}, {0, 1, 0}, {0, 1}}
    },
    // BOTTOM face (y-)
    std::array<Vertex, 4>{
        Vertex{{0, 0, 0}, {0, -1, 0}, {0, 0}},
        Vertex{{1, 0, 0}, {0, -1, 0}, {1, 0}},
        Vertex{{1, 0, 1}, {0, -1, 0}, {1, 1}},
        Vertex{{0, 0, 1}, {0, -1, 0}, {0, 1}}
    }
};

void MeshBuilder::emitFace(MeshData& mesh_data, Vec3i voxel_pos, VoxelFace face, const VoxelType& voxel) const {
    // Before pushing, record the starting index
    uint32_t startIndex = static_cast<uint32_t>(mesh_data.vertices.size());

    // Push the 4 vertices for this face
    for (const Vertex& v: VOXEL_FACE_VERTICES[face]) {
        mesh_data.vertices.push_back({
            Vec3f{voxel_pos} + v.pos,
            v.normal,
            v.uv
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

MeshData MeshBuilder::buildMesh(const Chunk& chunk) const {
    MeshData mesh_data;
    for (int x = 0; x < Chunk::WIDTH; ++x) {
	for (int y = 0; y < Chunk::HEIGHT; ++y) {
	    for (int z = 0; z < Chunk::DEPTH; ++z) {
		const VoxelType& voxel = chunk.getVoxel({ x, y, z });

		if (voxel.getId() == 0 || voxel.getName() == "air") {
		    continue;
		}

                for (int vfi = FRONT; vfi != BOTTOM; vfi++) {
                    VoxelFace face = static_cast<VoxelFace>(vfi);
                    Vec3i neighborPos = Vec3i{x, y, z} + direction(face);
                    VoxelType neighbor = chunk.getVoxelType(0);
                    //std::cout << "here in the mesh builder" << std::endl; TODO: rm this
                    if (chunk.positionInChunk(neighborPos)) {
                        neighbor = chunk.getVoxel(neighborPos);
                    }

                    if (neighbor.isTransparent()) {
                        emitFace(mesh_data, {x, y, z}, face, voxel);
                    }
                }
	    }
	}
    }
    return mesh_data;
}
