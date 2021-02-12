#include <istream>
#include <ostream>

#include "gl_helper.hpp"

#include "standard.hpp"


#if SDL_BYTEORDER == SDL_LIL_ENDIAN

    template <>
    uint8_t local_endian_to_big_endian(uint8_t value) {
        return value;
    }

    template <>
    uint8_t big_endian_to_local_endian(uint8_t value) {
        return value;
    }

    template <>
    uint32_t local_endian_to_big_endian(uint32_t value) {
        return SDL_Swap32(value);
    }

    template <>
    uint32_t big_endian_to_local_endian(uint32_t value) {
        return SDL_Swap32(value);
    }

    template <>
    uint64_t local_endian_to_big_endian(uint64_t value) {
        return SDL_Swap64(value);
    }

    template <>
    uint64_t big_endian_to_local_endian(uint64_t value) {
        return SDL_Swap64(value);
    }

    template <>
    int32_t local_endian_to_big_endian(int32_t value) {
        return static_cast<int32_t>(local_endian_to_big_endian<uint32_t>(static_cast<uint32_t>(value)));
    }

    template <>
    int32_t big_endian_to_local_endian(int32_t value) {
        return static_cast<int32_t>(big_endian_to_local_endian<uint32_t>(static_cast<uint32_t>(value)));
    }
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
    // If the native endian is already big_endian no conversions need to be done and only the type needs to be returned
    #define JUST_RETURN_VALUE(type) \
        template <> \
        type local_endian_to_big_endian(type value) { \
            return value; \
        } \
        \
        template <> \
        type big_endian_to_local_endian(type value) { \
            return value; \
        }

    JUST_RETURN_VALUE(uint8_t)
    JUST_RETURN_VALUE(uint32_t)
    JUST_RETURN_VALUE(int32_t)
    JUST_RETURN_VALUE(uint64_t)

    #undef JUST_RETURN_VALUE
#else
#error "SDL_BYTEORDER not defined"
#endif


bool ranges_overlap(float x0, float x0_offset, float x1, float x1_offset) {
    return (x0 <= x1 && x1 <= x0 + x0_offset) || (x0 <= x1 + x1_offset && x1 + x1_offset <= x0 + x0_offset);
}


AABB::AABB(float w, float h, float l) {
    width = w;
    height = h;
    length = l;
}

bool AABBIntersection(glm::vec3 pos0, AABB aabb0, glm::vec3 pos1, AABB aabb1) {
    return ranges_overlap(pos0.x,  aabb0.width, pos1.x, aabb1.width) &&
           ranges_overlap(pos0.y, aabb0.height, pos1.y, aabb1.height) &&
           ranges_overlap(pos0.z, aabb0.length, pos1.z, aabb1.length);
}