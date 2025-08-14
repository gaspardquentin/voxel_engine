#pragma once

#include <cmath>
#include <iostream>

template <typename T>
struct Vec2T {
    T x, y;

    Vec2T(): x(0), y(0) {}
    Vec2T(T x, T y): x(x), y(y) {}

    Vec2T operator+(const Vec2T& other) const { return {x + other.x, y + other.y}; }
    Vec2T operator-(const Vec2T& other) const { return {x - other.x, y - other.y}; }
    Vec2T operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vec2T operator/(float scalar) const { return {x / scalar, y / scalar}; }

    float dot(const Vec2T& other) const { return x * other.x + y * other.y; }
 
    float length() const { return std::sqrt(x * x + y * y); }

    friend std::ostream& operator<<(std::ostream& os, const Vec2T& vec) {
        os << "(" << vec.x << ", " << vec.y << ")";
        return os;
    } 
};

using Vec2 = Vec2T<float>;
using Vec2f = Vec2T<float>;
using Vec2i = Vec2T<int>;

template <typename T>
struct Vec3T {
    T x, y, z;
    Vec3T(): x(0), y(0), z(0) {}
    Vec3T(T x, T y, T z): x(x), y(y), z(z) {}

    template <typename U>
    explicit Vec3T(const Vec3T<U>& other)
        : x(static_cast<T>(other.x)),
        y(static_cast<T>(other.y)),
        z(static_cast<T>(other.z)) {}

    Vec2T<T> xy() const { return {x, y}; }

    Vec3T operator+(const Vec3T& other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3T operator-(const Vec3T& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3T operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }
    Vec3T operator/(float scalar) const { return {x / scalar, y / scalar, z / scalar}; }

    float dot(const Vec3T& other) const { return x * other.x + y * other.y + z * other.z; }

    Vec3T cross(const Vec3T& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    float length() const { return std::sqrt(x * x + y * y + z * z); }

    friend std::ostream& operator<<(std::ostream& os, const Vec3T& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }
};

using Vec3 = Vec3T<float>;
using Vec3f = Vec3T<float>;
using Vec3i = Vec3T<int>;
