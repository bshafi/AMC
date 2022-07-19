#include "../src/model.hpp"

#include <sstream>

#include "gtest/gtest.h"


TEST(MCModel, Test0) {
    std::stringstream ss;

    MCModel model(10, 0);
    model.serialize(ss);

    auto bob = MCModel::deserialize(ss);
    ASSERT_NE(bob, std::nullopt);

    ASSERT_EQ(bob->id, model.id);
}