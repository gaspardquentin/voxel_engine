#include <gtest/gtest.h>
#include "voxel_engine/math_utils.h"
#include <stdexcept>

// ===== Vec3T Arithmetic =====

TEST(Vec3Arithmetic, Addition) {
    Vec3f a{1.0f, 2.0f, 3.0f};
    Vec3f b{4.0f, 5.0f, 6.0f};
    Vec3f c = a + b;
    EXPECT_FLOAT_EQ(c.x, 5.0f);
    EXPECT_FLOAT_EQ(c.y, 7.0f);
    EXPECT_FLOAT_EQ(c.z, 9.0f);
}

TEST(Vec3Arithmetic, Subtraction) {
    Vec3f a{10.0f, 20.0f, 30.0f};
    Vec3f b{3.0f, 5.0f, 7.0f};
    Vec3f c = a - b;
    EXPECT_FLOAT_EQ(c.x, 7.0f);
    EXPECT_FLOAT_EQ(c.y, 15.0f);
    EXPECT_FLOAT_EQ(c.z, 23.0f);
}

TEST(Vec3Arithmetic, ScalarMultiplication) {
    Vec3f a{2.0f, 3.0f, 4.0f};
    Vec3f c = a * 3.0f;
    EXPECT_FLOAT_EQ(c.x, 6.0f);
    EXPECT_FLOAT_EQ(c.y, 9.0f);
    EXPECT_FLOAT_EQ(c.z, 12.0f);
}

TEST(Vec3Arithmetic, ScalarDivision) {
    Vec3f a{10.0f, 20.0f, 30.0f};
    Vec3f c = a / 2.0f;
    EXPECT_FLOAT_EQ(c.x, 5.0f);
    EXPECT_FLOAT_EQ(c.y, 10.0f);
    EXPECT_FLOAT_EQ(c.z, 15.0f);
}

TEST(Vec3Arithmetic, CompoundAddition) {
    Vec3f a{1.0f, 2.0f, 3.0f};
    a += Vec3f{10.0f, 20.0f, 30.0f};
    EXPECT_FLOAT_EQ(a.x, 11.0f);
    EXPECT_FLOAT_EQ(a.y, 22.0f);
    EXPECT_FLOAT_EQ(a.z, 33.0f);
}

// ===== Vec3T Dot Product =====

TEST(Vec3DotProduct, Perpendicular) {
    Vec3f a{1.0f, 0.0f, 0.0f};
    Vec3f b{0.0f, 1.0f, 0.0f};
    EXPECT_FLOAT_EQ(a.dot(b), 0.0f);
}

TEST(Vec3DotProduct, Parallel) {
    Vec3f a{2.0f, 0.0f, 0.0f};
    Vec3f b{3.0f, 0.0f, 0.0f};
    EXPECT_FLOAT_EQ(a.dot(b), 6.0f);
}

TEST(Vec3DotProduct, General) {
    Vec3f a{1.0f, 2.0f, 3.0f};
    Vec3f b{4.0f, 5.0f, 6.0f};
    EXPECT_FLOAT_EQ(a.dot(b), 32.0f); // 4+10+18
}

// ===== Vec3T Cross Product =====

TEST(Vec3CrossProduct, BasisVectors) {
    Vec3f x{1.0f, 0.0f, 0.0f};
    Vec3f y{0.0f, 1.0f, 0.0f};
    Vec3f z = x.cross(y);
    EXPECT_FLOAT_EQ(z.x, 0.0f);
    EXPECT_FLOAT_EQ(z.y, 0.0f);
    EXPECT_FLOAT_EQ(z.z, 1.0f);
}

TEST(Vec3CrossProduct, StaticVersion) {
    Vec3f a{1.0f, 0.0f, 0.0f};
    Vec3f b{0.0f, 1.0f, 0.0f};
    Vec3f c = Vec3f::cross(a, b);
    EXPECT_FLOAT_EQ(c.z, 1.0f);
}

TEST(Vec3CrossProduct, AntiCommutative) {
    Vec3f a{1.0f, 2.0f, 3.0f};
    Vec3f b{4.0f, 5.0f, 6.0f};
    Vec3f ab = a.cross(b);
    Vec3f ba = b.cross(a);
    EXPECT_FLOAT_EQ(ab.x, -ba.x);
    EXPECT_FLOAT_EQ(ab.y, -ba.y);
    EXPECT_FLOAT_EQ(ab.z, -ba.z);
}

// ===== Vec3T Length & Normalize =====

TEST(Vec3Length, UnitVector) {
    Vec3f a{1.0f, 0.0f, 0.0f};
    EXPECT_FLOAT_EQ(a.length(), 1.0f);
}

TEST(Vec3Length, General) {
    Vec3f a{3.0f, 4.0f, 0.0f};
    EXPECT_FLOAT_EQ(a.length(), 5.0f);
}

TEST(Vec3Normalize, UnitVector) {
    Vec3f a{5.0f, 0.0f, 0.0f};
    Vec3f n = Vec3f::normalize(a);
    EXPECT_FLOAT_EQ(n.x, 1.0f);
    EXPECT_FLOAT_EQ(n.y, 0.0f);
    EXPECT_FLOAT_EQ(n.z, 0.0f);
}

TEST(Vec3Normalize, General) {
    Vec3f a{3.0f, 4.0f, 0.0f};
    Vec3f n = Vec3f::normalize(a);
    EXPECT_NEAR(n.length(), 1.0f, 1e-5f);
}

TEST(Vec3Normalize, ZeroVector) {
    Vec3f zero{0.0f, 0.0f, 0.0f};
    Vec3f n = Vec3f::normalize(zero);
    EXPECT_FLOAT_EQ(n.x, 0.0f);
    EXPECT_FLOAT_EQ(n.y, 0.0f);
    EXPECT_FLOAT_EQ(n.z, 0.0f);
}

// ===== Vec3T Distance =====

TEST(Vec3Distance, SamePoint) {
    Vec3f a{5.0f, 5.0f, 5.0f};
    EXPECT_FLOAT_EQ(Vec3f::dist(a, a), 0.0f);
}

TEST(Vec3Distance, KnownDistance) {
    Vec3f a{0.0f, 0.0f, 0.0f};
    Vec3f b{3.0f, 4.0f, 0.0f};
    EXPECT_FLOAT_EQ(Vec3f::dist(a, b), 5.0f);
}

// ===== Vec3T fromString =====

TEST(Vec3FromString, WithParensAndSpaces) {
    Vec3i v = Vec3i::fromString("(1, 2, 3)");
    EXPECT_EQ(v.x, 1);
    EXPECT_EQ(v.y, 2);
    EXPECT_EQ(v.z, 3);
}

TEST(Vec3FromString, WithParensNoSpaces) {
    Vec3i v = Vec3i::fromString("(10,20,30)");
    EXPECT_EQ(v.x, 10);
    EXPECT_EQ(v.y, 20);
    EXPECT_EQ(v.z, 30);
}

TEST(Vec3FromString, WithoutParens) {
    Vec3i v = Vec3i::fromString("5,6,7");
    EXPECT_EQ(v.x, 5);
    EXPECT_EQ(v.y, 6);
    EXPECT_EQ(v.z, 7);
}

TEST(Vec3FromString, NegativeValues) {
    Vec3i v = Vec3i::fromString("(-1, -2, -3)");
    EXPECT_EQ(v.x, -1);
    EXPECT_EQ(v.y, -2);
    EXPECT_EQ(v.z, -3);
}

TEST(Vec3FromString, InvalidThrows) {
    EXPECT_THROW(Vec3i::fromString("not_a_vector"), std::invalid_argument);
    EXPECT_THROW(Vec3i::fromString("1,2"), std::invalid_argument);
    EXPECT_THROW(Vec3i::fromString(""), std::invalid_argument);
}

// ===== Vec2T fromString =====

TEST(Vec2FromString, Valid) {
    Vec2i v = Vec2i::fromString("(3, 7)");
    EXPECT_EQ(v.x, 3);
    EXPECT_EQ(v.y, 7);
}

TEST(Vec2FromString, InvalidThrows) {
    EXPECT_THROW(Vec2i::fromString("garbage"), std::invalid_argument);
}

// ===== Vec2T Chebyshev =====

TEST(Vec2Chebyshev, SamePoint) {
    Vec2i a{5, 5};
    EXPECT_EQ(Vec2i::chebyshev(a, a), 0);
}

TEST(Vec2Chebyshev, Diagonal) {
    Vec2i a{0, 0};
    Vec2i b{3, 7};
    EXPECT_EQ(Vec2i::chebyshev(a, b), 7); // max(3, 7)
}

TEST(Vec2Chebyshev, NegativeCoords) {
    Vec2i a{-2, -3};
    Vec2i b{1, 4};
    EXPECT_EQ(Vec2i::chebyshev(a, b), 7); // max(3, 7)
}

// ===== Equality =====

TEST(VecEquality, Vec3Equal) {
    Vec3f a{1.0f, 2.0f, 3.0f};
    Vec3f b{1.0f, 2.0f, 3.0f};
    EXPECT_TRUE(a == b);
}

TEST(VecEquality, Vec3NotEqual) {
    Vec3f a{1.0f, 2.0f, 3.0f};
    Vec3f b{1.0f, 2.0f, 4.0f};
    EXPECT_FALSE(a == b);
}

TEST(VecEquality, Vec2Equal) {
    Vec2i a{10, 20};
    Vec2i b{10, 20};
    EXPECT_TRUE(a == b);
}

// ===== Type Conversion =====

TEST(Vec3Conversion, FloatToInt) {
    Vec3f f{1.7f, 2.3f, 3.9f};
    Vec3i i{f};
    EXPECT_EQ(i.x, 1);
    EXPECT_EQ(i.y, 2);
    EXPECT_EQ(i.z, 3);
}

TEST(Vec3Conversion, IntToFloat) {
    Vec3i i{1, 2, 3};
    Vec3f f{i};
    EXPECT_FLOAT_EQ(f.x, 1.0f);
    EXPECT_FLOAT_EQ(f.y, 2.0f);
    EXPECT_FLOAT_EQ(f.z, 3.0f);
}
