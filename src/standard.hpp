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

template <typename A, typename B>
using pair_vector = std::vector<std::pair<A, B>>;


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

struct AABB {
    // width: x
    // height: y
    // length: z
    float width, height, length;
};

// expects that pos is in the bottom back left corner
bool AABBIntersection(glm::vec3 pos0, AABB aabb0, glm::vec3 pos1, AABB aabb1);

