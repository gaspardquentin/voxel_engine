#include <gtest/gtest.h>
#include "voxel_engine/server/entity_systems.h"
#include "voxel_engine/server/entity_components.h"
#include <entt/entt.hpp>

using namespace voxeng::server;

TEST(MovementSystem, BasicMovement) {
    entt::registry reg;
    auto ent = reg.create();
    reg.emplace<Position>(ent, Position{{0.0f, 0.0f, 0.0f}});
    reg.emplace<Velocity>(ent, Velocity{{10.0f, 0.0f, 0.0f}});

    movement_system::update(reg, 1.0f);

    auto& pos = reg.get<Position>(ent);
    EXPECT_FLOAT_EQ(pos.pos.x, 10.0f);
    EXPECT_FLOAT_EQ(pos.pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.pos.z, 0.0f);
}

TEST(MovementSystem, ZeroVelocity) {
    entt::registry reg;
    auto ent = reg.create();
    reg.emplace<Position>(ent, Position{{5.0f, 5.0f, 5.0f}});
    reg.emplace<Velocity>(ent, Velocity{{0.0f, 0.0f, 0.0f}});

    movement_system::update(reg, 1.0f);

    auto& pos = reg.get<Position>(ent);
    EXPECT_FLOAT_EQ(pos.pos.x, 5.0f);
    EXPECT_FLOAT_EQ(pos.pos.y, 5.0f);
    EXPECT_FLOAT_EQ(pos.pos.z, 5.0f);
}

TEST(MovementSystem, ZeroDeltaTime) {
    entt::registry reg;
    auto ent = reg.create();
    reg.emplace<Position>(ent, Position{{1.0f, 2.0f, 3.0f}});
    reg.emplace<Velocity>(ent, Velocity{{100.0f, 100.0f, 100.0f}});

    movement_system::update(reg, 0.0f);

    auto& pos = reg.get<Position>(ent);
    EXPECT_FLOAT_EQ(pos.pos.x, 1.0f);
    EXPECT_FLOAT_EQ(pos.pos.y, 2.0f);
    EXPECT_FLOAT_EQ(pos.pos.z, 3.0f);
}

TEST(MovementSystem, MultipleEntities) {
    entt::registry reg;

    auto e1 = reg.create();
    reg.emplace<Position>(e1, Position{{0.0f, 0.0f, 0.0f}});
    reg.emplace<Velocity>(e1, Velocity{{1.0f, 0.0f, 0.0f}});

    auto e2 = reg.create();
    reg.emplace<Position>(e2, Position{{10.0f, 10.0f, 10.0f}});
    reg.emplace<Velocity>(e2, Velocity{{0.0f, -5.0f, 0.0f}});

    auto e3 = reg.create();
    reg.emplace<Position>(e3, Position{{0.0f, 0.0f, 0.0f}});
    reg.emplace<Velocity>(e3, Velocity{{1.0f, 2.0f, 3.0f}});

    movement_system::update(reg, 2.0f);

    EXPECT_FLOAT_EQ(reg.get<Position>(e1).pos.x, 2.0f);
    EXPECT_FLOAT_EQ(reg.get<Position>(e2).pos.y, 0.0f);  // 10 + (-5)*2
    EXPECT_FLOAT_EQ(reg.get<Position>(e3).pos.z, 6.0f);   // 0 + 3*2
}

TEST(MovementSystem, NegativeVelocity) {
    entt::registry reg;
    auto ent = reg.create();
    reg.emplace<Position>(ent, Position{{10.0f, 10.0f, 10.0f}});
    reg.emplace<Velocity>(ent, Velocity{{-5.0f, -5.0f, -5.0f}});

    movement_system::update(reg, 2.0f);

    auto& pos = reg.get<Position>(ent);
    EXPECT_FLOAT_EQ(pos.pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.pos.y, 0.0f);
    EXPECT_FLOAT_EQ(pos.pos.z, 0.0f);
}

TEST(MovementSystem, FractionalDeltaTime) {
    entt::registry reg;
    auto ent = reg.create();
    reg.emplace<Position>(ent, Position{{0.0f, 0.0f, 0.0f}});
    reg.emplace<Velocity>(ent, Velocity{{100.0f, 200.0f, 300.0f}});

    movement_system::update(reg, 0.05f);

    auto& pos = reg.get<Position>(ent);
    EXPECT_FLOAT_EQ(pos.pos.x, 5.0f);
    EXPECT_FLOAT_EQ(pos.pos.y, 10.0f);
    EXPECT_FLOAT_EQ(pos.pos.z, 15.0f);
}

TEST(MovementSystem, EntityWithoutVelocity) {
    entt::registry reg;

    // Entity with only Position (no Velocity)
    auto ent = reg.create();
    reg.emplace<Position>(ent, Position{{5.0f, 5.0f, 5.0f}});

    // Should not crash or modify position
    movement_system::update(reg, 1.0f);

    auto& pos = reg.get<Position>(ent);
    EXPECT_FLOAT_EQ(pos.pos.x, 5.0f);
    EXPECT_FLOAT_EQ(pos.pos.y, 5.0f);
    EXPECT_FLOAT_EQ(pos.pos.z, 5.0f);
}
