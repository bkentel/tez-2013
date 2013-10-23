#include "pch.hpp"

#include <gtest/gtest.h>
#include "game/grid2d.hpp"

TEST(Grid2d, Sanity) {
    using grid = tez::grid2d<std::string>;

    auto g = grid(10, 10);
    std::string const s0 {};

    for (auto const& i : g) {
        ASSERT_EQ(s0, i);
    }
}
