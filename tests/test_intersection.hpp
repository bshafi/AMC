#pragma once

#include "../src/standard.hpp"

#include "gtest/gtest.h"

TEST(TestIntersection, Test) {
    BoundingBox bb0 = {
        vec3{ 0, 0, 0 },
        AABB{ 1, 1, 1 }
    };
    BoundingBox bb1 = {
        vec3( 0, -1, 0 ),
        AABB{ 1, 1, 1 }
    };
    ASSERT_FALSE(intersects(bb0, bb1));
}