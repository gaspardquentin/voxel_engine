#include <gtest/gtest.h>

#include "voxel_engine/network/local_transport.h"
#include "voxel_engine/server/chunk_generators.h"
#include "voxel_engine/server/entity_components.h"
#include "voxel_engine/server/network/local_server_connection.h"
#include "voxel_engine/server/world.h"
#include "voxel_engine/thread_pool.h"
#include "voxel_engine/voxel_types.h"
#include "voxel_engine/world_coords.h"

#include <chrono>
#include <map>
#include <thread>

using namespace voxeng;
using namespace voxeng::server;

namespace {

constexpr uint8_t RENDER_DISTANCE = 3;
constexpr size_t RING_SIZE = (2 * RENDER_DISTANCE + 1) * (2 * RENDER_DISTANCE + 1);

struct StreamingFixture {
    network::LocalTransport transport;
    network::LocalServerConnection connection{transport};
    ThreadPool pool{2};
    World world{connection, DEFAULT_VOXEL_TYPES, 0, std::make_unique<FlatGenerator>(), pool, false};
    entt::entity player;

    StreamingFixture() {
        world.playerJoin({1, "test"}, {0.f, 0.f, 0.f}, RENDER_DISTANCE);
        player = world.getPlayerEntities().at(1);
    }

    void moveTo(WorldCoord pos) {
        world.getRegistry().get<Position>(player).pos = pos;
        world.updateChunks();
        world.update();
    }

    // pumps World::update until no chunk event shows up for a while
    void pumpUntilIdle() {
        auto last_change = std::chrono::steady_clock::now();
        size_t last_count = transport.events.size();
        while (std::chrono::steady_clock::now() - last_change < std::chrono::milliseconds(200)) {
            world.update();
            if (transport.events.size() != last_count) {
                last_count = transport.events.size();
                last_change = std::chrono::steady_clock::now();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    std::map<std::pair<int32_t, int32_t>, int> chunkDataCounts() {
        std::map<std::pair<int32_t, int32_t>, int> counts;
        while (!transport.events.empty()) {
            if (auto* evt = std::get_if<network::ChunkDataEvent>(&transport.events.front())) {
                counts[{evt->id.x, evt->id.y}]++;
            }
            transport.events.pop();
        }
        return counts;
    }
};

}

TEST(ChunkStreaming, LoadsFullRingOnce) {
    StreamingFixture f;
    f.moveTo({0.f, 0.f, 0.f});
    f.pumpUntilIdle();

    auto counts = f.chunkDataCounts();
    EXPECT_EQ(counts.size(), RING_SIZE);
    for (auto& [id, count]: counts) {
        EXPECT_EQ(count, 1) << "chunk (" << id.first << ", " << id.second << ") sent " << count << " times";
    }
    EXPECT_EQ(f.world.getChunks().size(), RING_SIZE);
}

TEST(ChunkStreaming, NoDuplicatesWhenMovingBackAndForth) {
    StreamingFixture f;
    // cross a chunk boundary back and forth before generation results are collected
    for (int i = 0; i < 10; i++) {
        f.moveTo({0.f, 0.f, 0.f});
        f.moveTo({static_cast<float>(CHUNK_WIDTH) + 1.f, 0.f, 0.f});
    }
    f.pumpUntilIdle();

    for (auto& [id, count]: f.chunkDataCounts()) {
        EXPECT_EQ(count, 1) << "chunk (" << id.first << ", " << id.second << ") sent " << count << " times";
    }
}

TEST(ChunkStreaming, DropsChunksOutOfRange) {
    StreamingFixture f;
    f.moveTo({0.f, 0.f, 0.f});
    // teleport far away before anything is collected
    f.moveTo({1000.f * CHUNK_WIDTH, 0.f, 0.f});
    f.pumpUntilIdle();

    ChunkID center = getChunkId(WorldCoord{1000.f * CHUNK_WIDTH, 0.f, 0.f});
    for (auto& [id, chunk]: f.world.getChunks()) {
        EXPECT_LE(ChunkID::chebyshev(center, id), RENDER_DISTANCE + 2);
    }
    EXPECT_EQ(f.world.getChunks().size(), RING_SIZE);
}

TEST(ChunkStreaming, WorldDestroyedWithTasksInFlight) {
    network::LocalTransport transport;
    network::LocalServerConnection connection{transport};
    ThreadPool pool{2};
    for (int i = 0; i < 20; i++) {
        World world{connection, DEFAULT_VOXEL_TYPES, 0, std::make_unique<FlatGenerator>(), pool, false};
        world.playerJoin({1, "test"}, {0.f, 0.f, 0.f}, RENDER_DISTANCE);
        world.updateChunks();
        world.update(); // submits, then the world is destroyed right away
    }
    SUCCEED();
}
