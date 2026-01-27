#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>

/* =============== Helper Functions ===================== */
namespace voxeng {
    namespace math {
        inline double rad_to_deg(double radians) {
            return radians * (180.0 / M_PI);
        }
        inline double deg_to_rad(double deg) {
            return deg * M_PI / 180;
        }
    }
}

/* ================ Vectors Data structures ============== */
template <typename T>
struct Vec2T {
    T x, y;

    Vec2T(): x(0), y(0) {}
    Vec2T(T x, T y): x(x), y(y) {}

    Vec2T operator+(const Vec2T& other) const { return {x + other.x, y + other.y}; }
    Vec2T operator+=(const Vec2T& other) { 
        x += other.x;
        y += other.y;
        return *this;
    }
    Vec2T operator-(const Vec2T& other) const { return {x - other.x, y - other.y}; }
    Vec2T operator-=(const Vec2T& other) { 
        x -= other.x;
        y -= other.y;
        return *this;
    }

    bool operator==(const Vec2T& other) const {
        return x == other.x && y == other.y;
    }

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
//Note: doing this limits the amount of chunks of the engine, but this might be 
//enough and makes the hashing function more efficient
//This might change in the future.
using ChunkID = Vec2T<int32_t>;



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
    Vec3T operator+=(const Vec3T& other) { 
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    bool operator==(const Vec3T& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    Vec3T operator-(const Vec3T& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3T operator-=(const Vec3T& other) { 
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
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

    static Vec3T cross(const Vec3T& v1, const Vec3T& v2) {
        return {
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        };
    }

    float length() const { return std::sqrt(x * x + y * y + z * z); }
    static Vec3T normalize(const Vec3T& vec) {
        if (vec.length() == 0) {
            return {0, 0, 0};
        }
        return {
            vec.x / vec.length(),
            vec.y / vec.length(),
            vec.z / vec.length(),
        };
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec3T& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }
};

using Vec3 = Vec3T<float>;
using Vec3f = Vec3T<float>;
using Vec3i = Vec3T<int>;
using Vec3u = Vec3T<unsigned int>;

// Note: Needed for obj file parsing
// This should be interpreted as {v_idx, vt_idx, vn_idx}
using ObjVertexIndex = Vec3T<int>;


// Hash for the chunk id type (needed to store the chunks in the unordered map 
// accessed by their id)
// And Hash needed for the de-duplication algorithm in the obj file parsing
namespace std {
    template<>
    struct hash<ChunkID> {
        size_t operator()(const ChunkID& chunk_id) const noexcept {
            return (static_cast<uint64_t>(static_cast<uint32_t>(chunk_id.x)) << 32) | static_cast<uint32_t>(chunk_id.y);
        }
    };

    template<>
    struct hash<ObjVertexIndex> {
        size_t operator()(const ObjVertexIndex& k) const {
            return ((std::hash<int>()(k.x) ^
                    (std::hash<int>()(k.y) << 1)) >> 1) ^
                    (std::hash<int>()(k.z) << 1);
        }
    };
}

