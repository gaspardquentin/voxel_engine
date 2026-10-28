#include <gtest/gtest.h>
#include "voxel_engine/collision.h"

using namespace voxeng;
using collision::isColliding;

// Solid ground: every voxel with y < 0 (top surface at y = 0).
static const auto FLOOR = [](Vec3i p) { return p.y < 0; };
// Solid wall: every voxel with x >= 5 (face at x = 5).
static const auto WALL = [](Vec3i p) { return p.x >= 5; };
static const auto EMPTY = [](Vec3i) { return false; };

static const Vec3f SIZE{0.6f, 1.8f, 0.6f};

TEST(CollisionTest, EmptyWorldNeverCollides) {
    EXPECT_FALSE(isColliding({0.f, 0.f, 0.f}, SIZE, EMPTY));
}

TEST(CollisionTest, EmbeddedInSolidCollides) {
    EXPECT_TRUE(isColliding({0.f, -0.5f, 0.f}, SIZE, FLOOR));
}

TEST(CollisionTest, StandingOnFloorDoesNotCollide) {
    EXPECT_FALSE(isColliding({0.2f, 0.f, 0.2f}, SIZE, FLOOR));
}

TEST(CollisionTest, SlightlyBelowFloorSurfaceCollides) {
    EXPECT_TRUE(isColliding({0.2f, -0.01f, 0.2f}, SIZE, FLOOR));
}

TEST(CollisionTest, FlushAgainstWallDoesNotCollide) {
    // Box spans x in [4.4, 5.0]: touches the wall face but does not enter it.
    EXPECT_FALSE(isColliding({4.4f, 0.f, 0.f}, SIZE, WALL));
}

TEST(CollisionTest, PenetratingWallCollides) {
    EXPECT_TRUE(isColliding({4.5f, 0.f, 0.f}, SIZE, WALL));
}

TEST(CollisionTest, IntegerAlignedUnitBoxDoesNotCollideWithNeighbour) {
    // Unit box exactly on voxel (0,0,0); only that voxel is solid-free, neighbours are solid.
    auto only_origin_free = [](Vec3i p) { return !(p.x == 0 && p.y == 0 && p.z == 0); };
    EXPECT_FALSE(isColliding({0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, only_origin_free));
}

TEST(CollisionTest, BoxSpanningTwoVoxelsChecksBoth) {
    auto single = [](Vec3i p) { return p == Vec3i(1, 0, 0); };
    EXPECT_TRUE(isColliding({0.8f, 0.f, 0.f}, {0.5f, 0.5f, 0.5f}, single));
    EXPECT_FALSE(isColliding({0.2f, 0.f, 0.f}, {0.5f, 0.5f, 0.5f}, single));
}

TEST(CollisionTest, NegativeCoordinatesUseFloor) {
    // Voxel (-1, 0, 0) covers x in [-1, 0). A box at x = -0.3 must hit it.
    auto single = [](Vec3i p) { return p == Vec3i(-1, 0, 0); };
    EXPECT_TRUE(isColliding({-0.3f, 0.f, 0.f}, {0.2f, 0.2f, 0.2f}, single));
    // Same box in positive space must not (truncation would wrongly map -0.3 to 0).
    auto zero = [](Vec3i p) { return p == Vec3i(0, 0, 0); };
    EXPECT_FALSE(isColliding({-0.3f, 0.f, 0.f}, {0.2f, 0.2f, 0.2f}, zero));
}

TEST(CollisionTest, NegativeFloorStandingAndFlush) {
    auto neg_floor = [](Vec3i p) { return p.y < -10; };
    EXPECT_FALSE(isColliding({-7.5f, -10.f, -3.2f}, SIZE, neg_floor));
    EXPECT_TRUE(isColliding({-7.5f, -10.01f, -3.2f}, SIZE, neg_floor));
}
