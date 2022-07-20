#include "../src/model.hpp"

#include <sstream>

#include "gtest/gtest.h"


TEST(MCModelPart, Test0) {
    std::stringstream ss;

    MCModelPart model(10, 0);
    model.serialize(ss);

    auto bob = MCModelPart::deserialize(ss);
    ASSERT_NE(bob, std::nullopt);

    ASSERT_EQ(bob->id, model.id);
}