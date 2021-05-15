#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <map>
#include <optional>
#include <iostream>

//#define MANUAL_LEAK_CHECK

template <typename A, typename B>
using pair_vector = std::vector<std::pair<A, B>>;

template <uint32_t WidthX, uint32_t HeightY, typename T>
using Array2d = std::array<std::array<T, HeightY>, WidthX>;

template <uint32_t WidthX, uint32_t HeightY, uint32_t LengthZ, typename T>
using Array3d = std::array<std::array<std::array<T, LengthZ>, HeightY>, WidthX>;


template <uint32_t WidthX, uint32_t HeightY, uint32_t LengthZ, typename T>
constexpr glm::ivec3 array3d_bounds(const Array3d<WidthX, HeightY, LengthZ, T> &blocks) {
    return glm::ivec3(WidthX, HeightY, LengthZ);
}

bool array3d_index_in_bounds(const glm::ivec3 &index, const glm::ivec3 &bounds);

void array3d_iterate_index(glm::ivec3 &pos, const glm::ivec3 &bounds);

// This forces the static_assert to evaluate on the type argument rather than evaluating
// all the time
// This allows the enforcement of certain types being implemented
template <typename T>
struct always_false {
    static const bool value = false;
};

template <typename T>
T local_endian_to_big_endian(T value);

template <typename T>
T big_endian_to_local_endian(T value);

// Will throw exceptions when an error occurs
template <typename T>
T read_binary(std::istream &is) {

    T val;
    is.read(reinterpret_cast<char*>(&val), sizeof(T));
    val = big_endian_to_local_endian<T>(val);
    return val;
}

template <typename T>
std::ostream& write_binary(std::ostream &os, const T &val) {
    T val_copy = local_endian_to_big_endian(val);
    return os.write(reinterpret_cast<char*>(&val_copy), sizeof(T));
}

struct frect {
    float x, y, w, h;

    explicit operator glm::vec4() const;
    bool contains(const glm::vec2 &pos) const;
    glm::vec2 top_left() const;
    glm::vec2 bottom_right() const;
    frect inset_by(const float f) const;
    frect apply_equivalent_transformation(const frect &pre, const frect &post) const;
};

frect vec4_to_frect(const glm::vec4 &);
frect min_max_scaling(const frect &inner, const frect &outer);
frect apply_equivalent_transformation(const frect &pre_transform, const frect &post_transform, const frect &inner);

std::ostream& operator<<(std::ostream &, const frect&);

struct color {
    float r, g, b, a;

    explicit operator glm::vec4() const;
};

struct AABB {
    // width: x
    // height: y
    // length: z
    float width, height, length;

    AABB(float = 0, float = 0, float = 0);
};

// expects that pos is in the bottom back left corner
bool AABBIntersection(glm::vec3 pos0, AABB aabb0, glm::vec3 pos1, AABB aabb1);


struct Handle {
public:
    bool operator<(const Handle &lhs) const {
        return h_id < lhs.h_id;
    }
    bool operator<=(const Handle &lhs) const {
        return h_id <= lhs.h_id;
    }
    bool operator>(const Handle &lhs) const {
        return h_id > lhs.h_id;
    }
    bool operator>=(const Handle &lhs) const {
        return h_id >= lhs.h_id;
    }
    bool operator==(const Handle &lhs) const {
        return h_id == lhs.h_id;
    }
    bool operator!=(const Handle &lhs) const {
        return h_id != lhs.h_id;
    }
    uint32_t id() const {
        return h_id;
    }
protected:
    Handle(uint32_t id) {
        this->h_id = id;
    }
private:
    uint32_t h_id;
};

