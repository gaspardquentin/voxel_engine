#include <gtest/gtest.h>

#include "rendering/world_mesh_builder.h"
#include "voxel_engine/chunk.h"
#include "voxel_engine/voxel_types.h"

#include <algorithm>
#include <random>
#include <tuple>

using namespace voxeng;
using namespace voxeng::client;

namespace {

// Face visibility rules of the original (pre-optimization) mesher, expressed per voxel face.
// Returns, for every emitted face, (x, y, z, face, voxel id).
using Face = std::tuple<unsigned, unsigned, unsigned, int, VoxelID>;

std::vector<Face> referenceFaces(const Chunk& chunk) {
    static const int DIRS[6][3] = {{0, 0, 1}, {0, 0, -1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
    std::vector<Face> faces;
    for (unsigned x = 0; x < Chunk::WIDTH; ++x) {
        for (unsigned y = 0; y < Chunk::HEIGHT; ++y) {
            for (unsigned z = 0; z < Chunk::DEPTH; ++z) {
                const VoxelType& voxel = chunk.getVoxel({x, y, z});
                if (voxel.getId() == 0 || voxel.getName() == "air") continue;
                for (int f = 0; f < 6; f++) {
                    ChunkCoord n{x + DIRS[f][0], y + DIRS[f][1], z + DIRS[f][2]};
                    VoxelType neighbor = chunk.getVoxelType(0);
                    if (chunk.positionInChunk(n)) neighbor = chunk.getVoxel(n);
                    bool emit = neighbor.isTransparent();
                    if (voxel.isTransparent() && voxel.getId() == neighbor.getId()) emit = false;
                    if (emit) faces.emplace_back(x, y, z, f, voxel.getId());
                }
            }
        }
    }
    std::sort(faces.begin(), faces.end());
    return faces;
}

// Recovers the faces from a built mesh (4 vertices per face, normal gives the face).
std::vector<Face> meshFaces(const MeshData& mesh, Vec3f world_pos) {
    static const int DIRS[6][3] = {{0, 0, 1}, {0, 0, -1}, {-1, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
    std::vector<Face> faces;
    for (size_t i = 0; i < mesh.vertices.size(); i += 4) {
        // voxel origin is the min corner of the 4 vertices
        float mx = 1e9f, my = 1e9f, mz = 1e9f;
        for (size_t k = 0; k < 4; k++) {
            mx = std::min(mx, mesh.vertices[i + k].pos.x);
            my = std::min(my, mesh.vertices[i + k].pos.y);
            mz = std::min(mz, mesh.vertices[i + k].pos.z);
        }
        const Vertex& v = mesh.vertices[i];
        int face = -1;
        for (int f = 0; f < 6; f++) {
            if (v.normal.x == DIRS[f][0] && v.normal.y == DIRS[f][1] && v.normal.z == DIRS[f][2]) face = f;
        }
        // faces on the + side sit one voxel further along their axis
        if (face == 0) mz -= 1.f;
        if (face == 3) mx -= 1.f;
        if (face == 4) my -= 1.f;
        faces.emplace_back(
            static_cast<unsigned>(mx - world_pos.x),
            static_cast<unsigned>(my - world_pos.y),
            static_cast<unsigned>(mz - world_pos.z),
            face,
            static_cast<VoxelID>(v.uv.z));
    }
    std::sort(faces.begin(), faces.end());
    return faces;
}

void expectSameAsReference(std::array<VoxelID, CHUNK_SIZE> data) {
    Vec3f world_pos{-32.f, 0.f, 48.f};
    Chunk chunk{DEFAULT_VOXEL_TYPES, world_pos, std::move(data)};
    WorldMeshBuilder builder;
    builder.setVoxelTypes(DEFAULT_VOXEL_TYPES);

    MeshData mesh = builder.buildMesh(chunk);
    ASSERT_EQ(mesh.indices.size(), mesh.vertices.size() / 4 * 6);
    EXPECT_EQ(meshFaces(mesh, world_pos), referenceFaces(chunk));
}

}

TEST(Mesher, EmptyChunk) {
    std::array<VoxelID, CHUNK_SIZE> data{};
    expectSameAsReference(data);
}

TEST(Mesher, FullChunk) {
    std::array<VoxelID, CHUNK_SIZE> data;
    data.fill(3);
    expectSameAsReference(data);
}

TEST(Mesher, WaterAndGlassSlabs) {
    std::array<VoxelID, CHUNK_SIZE> data{};
    for (unsigned y = 0; y < Chunk::HEIGHT; y++)
        for (unsigned z = 0; z < Chunk::DEPTH; z++)
            for (unsigned x = 0; x < Chunk::WIDTH; x++)
                data[Chunk::linearIndex({x, y, z})] = y < 4 ? 3 : y < 8 ? 8 : y < 9 ? 10 : 0;
    expectSameAsReference(data);
}

TEST(Mesher, RandomChunks) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(DEFAULT_VOXEL_TYPES.size()) - 1);
    for (int i = 0; i < 20; i++) {
        std::array<VoxelID, CHUNK_SIZE> data;
        for (auto& v: data) v = static_cast<VoxelID>(dist(rng));
        expectSameAsReference(data);
    }
}
