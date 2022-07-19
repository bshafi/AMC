#include <istream>
#include <ostream>
#include <iomanip>

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

    template <>
    float local_endian_to_big_endian(float value) {
        uint32_t h_value;
        static_assert(sizeof(value) == sizeof(h_value));
        memcpy(&h_value, &value, sizeof(value));

        h_value = local_endian_to_big_endian<uint32_t>(h_value);
        memcpy(&value, &h_value, sizeof(value));

        return value;
    }

    template <>
    float big_endian_to_local_endian(float value) {
        uint32_t h_value;
        static_assert(sizeof(value) == sizeof(h_value));
        memcpy(&h_value, &value, sizeof(value));

        h_value = big_endian_to_local_endian<uint32_t>(h_value);
        memcpy(&value, &h_value, sizeof(value));

        return value;
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
    JUST_RETURN_VALUE(uint64_t)

    JUST_RETURN_VALUE(int8_t)
    JUST_RETURN_VALUE(int32_t)
    JUST_RETURN_VALUE(int64_t)
    JUST_RETURN_VALUE(float)

    #undef JUST_RETURN_VALUE
#else
#error "SDL_BYTEORDER not defined"
#endif


float roundup(float x) {
    return floor(x + 0.001f) + 1;
}
glm::vec3 roundup(const glm::vec3 &a) {
    return glm::vec3(roundup(a.x), roundup(a.y), roundup(a.z));
}

bool ranges_overlap(float x0, float x0_offset, float x1, float x1_offset) {
    return (x0 < x1 && x1 < x0 + x0_offset) || (x0 < x1 + x1_offset && x1 + x1_offset < x0 + x0_offset);
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

bool BoundingBox::contains(const glm::vec3 &point) const {
    const bool x_overlap = (this->pos.x <= point.x) && (point.x <= this->pos.x + this->aabb.width);
    const bool y_overlap = (this->pos.y <= point.y) && (point.y <= this->pos.y + this->aabb.height);
    const bool z_overlap = (this->pos.z <= point.z) && (point.z <= this->pos.z + this->aabb.length);
    return x_overlap && y_overlap && z_overlap;
}
std::array<vec3, 8> BoundingBox::corners() const {
    return {
        this->pos,
        this->pos + vec3(this->aabb.width, 0, 0),
        this->pos + vec3(0, this->aabb.height, 0),
        this->pos + vec3(this->aabb.width, this->aabb.height, 0),
        this->pos + vec3(0, 0, this->aabb.length),
        this->pos + vec3(this->aabb.width, 0, this->aabb.length),
        this->pos + vec3(0, this->aabb.height, this->aabb.length),
        this->pos + vec3(this->aabb.width, this->aabb.height, this->aabb.length)
    };
}

BoundingBox BoundingBox::union_box(const BoundingBox &r) const {
    auto l_corners = this->corners();
    auto r_corners = r.corners();
    vec3 points[] = {
        l_corners[0],
        l_corners[1],
        l_corners[2],
        l_corners[3],
        l_corners[4],
        l_corners[5],
        l_corners[6],
        l_corners[7],
        r_corners[0],
        r_corners[1],
        r_corners[2],
        r_corners[3],
        r_corners[4],
        r_corners[5],
        r_corners[6],
        r_corners[7],
    };
    float x_min = INFINITY, x_max = -INFINITY;
    float y_min = INFINITY, y_max = -INFINITY;
    float z_min = INFINITY, z_max = -INFINITY;

    for (size_t i = 0; i < sizeof(points) / sizeof(points[0]); ++i) {
        x_min = std::min(x_min, points[i].x);
        x_max = std::max(x_max, points[i].x);
        y_min = std::min(y_min, points[i].y);
        y_max = std::max(y_max, points[i].y);
        z_min = std::min(z_min, points[i].z);
        z_max = std::max(z_max, points[i].z);
    }

    return BoundingBox{
        {
            x_min,
            y_min,
            z_min
        },
        {
            x_max - x_min,
            y_max - y_min,
            z_max - z_min
        }
    };
}
namespace std {
    static uint32_t i32_to_u32(int32_t x) {
        return static_cast<uint32_t>(static_cast<int64_t>(x) - static_cast<int64_t>(INT32_MIN));
    }
    size_t hash<glm::ivec2>::operator()(const glm::ivec2 &pos) const {
        uint64_t x = i32_to_u32(pos.x); 
        uint64_t y = i32_to_u32(pos.y);
        return static_cast<size_t>((x << 32) | (y));
    }
}

template <>
bool intersects<BoundingBox, BoundingBox>(const BoundingBox &a, const BoundingBox &b) {
    return AABBIntersection(a.pos, a.aabb, b.pos, b.aabb);
}

frect::operator glm::vec4() const {
    return glm::vec4(x, y, w, h);
}
bool frect::contains(const glm::vec2 &pos) const {
    return this->x <= pos.x && pos.x <= this->x + this->w && this->y <= pos.y && pos.y <= this->y + this->h;
}

glm::vec2 frect::top_left() const {
    return glm::vec2(x, y);
}
glm::vec2 frect::bottom_right() const {
    return glm::vec2(x, y) + glm::vec2(w, h);
}
frect frect::inset_by(const float f) const {
    assert(0 <= f && f <= 1);
    const float k = fminf(w, h) * f;
    return frect{
        x + k / 2,
        y + k / 2,
        w - k,
        h - k
    };
}
frect frect::apply_equivalent_transformation(const frect &pre, const frect &post) const {
    return ::apply_equivalent_transformation(pre, post, *this);
}
frect vec4_to_frect(const glm::vec4 &v) {
    return frect{ v.x, v.y, v.z, v.w };
}

frect min_max_scaling(const frect &inner, const frect &outer, const VAlignment &valign, const HAlignment &halign) {
    const float scale = std::min(outer.w / inner.w, outer.h / inner.h);
    const float new_width = inner.w * scale;
    const float new_height = inner.h * scale;

    static_assert(static_cast<uint32_t>(VAlignment::Top) == 0);
    static_assert(static_cast<uint32_t>(VAlignment::Center) == 1);
    static_assert(static_cast<uint32_t>(VAlignment::Bottom) == 2);

    static_assert(static_cast<uint32_t>(HAlignment::Left) == 0);
    static_assert(static_cast<uint32_t>(HAlignment::Center) == 1);
    static_assert(static_cast<uint32_t>(HAlignment::Right) == 2);

    const float new_x = outer.x + static_cast<uint32_t>(halign) * (outer.w - new_width) / 2.0f;

    const float new_y = outer.y + static_cast<uint32_t>(valign) * (outer.h - new_height) / 2.0f;

    return frect {
        new_x,
        new_y,
        new_width,
        new_height
    };
}
frect apply_equivalent_transformation(const frect &pre_transform, const frect &post_transform, const frect &inner) {
    // ul -> upper left
    // br -> upper right
    const glm::vec2 pre_transform_ul = glm::vec2(pre_transform.x, pre_transform.y);
    const glm::vec2 pre_transform_br = pre_transform_ul + glm::vec2(pre_transform.w, pre_transform.h);

    const glm::vec2 post_transform_ul = glm::vec2(post_transform.x, post_transform.y);
    const glm::vec2 post_transform_br = post_transform_ul + glm::vec2(post_transform.w, post_transform.h);

    const glm::vec2 pre_inner_ul = glm::vec2(inner.x, inner.y);
    const glm::vec2 pre_inner_br = pre_inner_ul + glm::vec2(inner.w, inner.h);

    // post = A * pre + B
    const glm::vec2 a = (post_transform_br - post_transform_ul) / (pre_transform_br - pre_transform_ul);
    const glm::vec2 b = post_transform_ul - a * pre_transform_ul;
    if (glm::length(b - (post_transform_br - a * pre_transform_br)) >= 0.0001f) {
        std::cout << "Off by " << glm::length(b - (post_transform_br - a * pre_transform_br)) << std::endl;
        assert(false);
    }

    const glm::vec2 post_inner_ul = a * pre_inner_ul + b;
    const glm::vec2 post_inner_br = a * pre_inner_br + b;
    const glm::vec2 post_width_height = post_inner_br - post_inner_ul;
    return frect{ post_inner_ul.x, post_inner_ul.y, post_width_height.x, post_width_height.y };
}
std::ostream& operator<<(std::ostream &os, const frect &rect) {
    const uint32_t width = 5;
    return os << "{ "  << std::setw(width) << rect.x << ", " << std::setw(width) << rect.y << ", " << std::setw(width) << rect.w << ", " << std::setw(width) << rect.h << " }";
}


color::operator glm::vec4() const {
    return glm::vec4(r, g, b, a);
}

template <>
std::optional<float> Ray::cast(const Plane &plane, const float length) const {
    const float denominator = glm::dot(direction, plane.normal);
    if (fabs(denominator) <= 0.0001f) {
        return std::nullopt;
    }
    const float t = glm::dot(plane.offset - endpoint, plane.normal) / denominator;
    if (t < 0.0f || t > length) {
        return std::nullopt;
    } else {
        return t;
    }
}
template <>
std::optional<float> Ray::cast(const BoundingBox &box, const float length) const {
    const glm::vec3 back_bottom_left = box.pos;
    const glm::vec3 top_front_right = box.pos + glm::vec3(box.aabb.width, box.aabb.height, box.aabb.length);

    const glm::vec3 bbl_t = (back_bottom_left - endpoint) / direction;

    const glm::vec3 tfr_t = (top_front_right - endpoint) / direction;

    const float ts[] = { bbl_t.x, bbl_t.y, bbl_t.z, tfr_t.x, tfr_t.y, tfr_t.z };

    const float ERROR = 0.001f;
    const BoundingBox loose_box = BoundingBox{
        box.pos - glm::vec3(ERROR, ERROR, ERROR), 
        {
            box.aabb.width + 2 * ERROR,
            box.aabb.height + 2 * ERROR,
            box.aabb.length + 2 * ERROR,
        } 
    };
    std::optional<float> t;
    for (int i = 0; i < 6; ++i) {
        if (isnan(ts[i]) || isinf(ts[i]) || ts[i] < 0 || ts[i] > length) {
            continue;
        }
        if (loose_box.contains(ts[i] * direction + endpoint)) {
            if (t.has_value()) {
                t = std::min(ts[i], *t);
            } else {
                t = ts[i];
            }
        }
    }

    return t;
}

template <>
bool intersects<Ray, Plane>(const Ray &ray, const Plane &plane) {
    return ray.cast(plane).has_value();
}

template <>
bool intersects<Ray, BoundingBox>(const Ray &ray, const BoundingBox &box) {
    return ray.cast(box).has_value();
}

bool Plane::point_in_half_space(const glm::vec3 &pos) const {
    return glm::dot(offset - pos, normal) >= 0.0f;
}