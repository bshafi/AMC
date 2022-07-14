#include "test_raycast.hpp"
#include "test_intersection.hpp"
#include "test_save_file.hpp"
#include "test_atomic_ring_buffer.hpp"

#include "gtest/gtest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}