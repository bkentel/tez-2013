#include "pch.hpp"

#include <gtest/gtest.h>
#include "game/grid2d.hpp"

TEST(Grid2d, Sanity) {
    using grid = tez::grid2d<std::string>;

    auto const w = 10;
    auto const h = 15;
    std::string const value = "test";

    auto g = grid(w, h, value);

    //dimensions
    ASSERT_EQ(g.width(),  w);
    ASSERT_EQ(g.height(), h);

    //check initial values and iteration count
    auto n = 0;
    for (auto const& i : g) {
        ASSERT_EQ(i, value);
        n++;
    }
    ASSERT_EQ(n, g.size());

    //check indicies match expected for iteration
    auto it  = g.cbegin();
    auto const end = g.cend();
    for (auto iy = 0; iy < h; ++iy) {
        for (auto ix = 0; ix < w; ++ix) {
            auto const i = it->i;
            ASSERT_EQ(i.x, ix);
            ASSERT_EQ(i.y, iy);

            ++it;
        }
    }
    ASSERT_EQ(it, end);
}

TEST(Grid2d, Swap) {
    using grid = tez::grid2d<std::string>;

    auto const wa = 10;
    auto const ha = 15;
    std::string const value_a = "test a";

    auto const wb = 20;
    auto const hb = 25;
    std::string const value_b = "test b";

    auto grid_a = grid(wa, ha, value_a);
    auto grid_b = grid(wb, hb, value_b);

    using std::swap;
    swap(grid_a, grid_b);

    ASSERT_EQ(grid_a.width(), wb);
    ASSERT_EQ(grid_a.height(), hb);
    for (auto const& i : grid_a) ASSERT_EQ(i, value_b);

    ASSERT_EQ(grid_b.width(), wa);
    ASSERT_EQ(grid_b.height(), ha);
    for (auto const& i : grid_b) ASSERT_EQ(i, value_a);

    grid grid_aa = std::move(grid_b);
    grid grid_bb = std::move(grid_a);

    ASSERT_EQ(grid_aa.width(), wa);
    ASSERT_EQ(grid_aa.height(), ha);
    for (auto const& i : grid_aa) ASSERT_EQ(i, value_a);

    ASSERT_EQ(grid_bb.width(), wb);
    ASSERT_EQ(grid_bb.height(), hb);
    for (auto const& i : grid_bb) ASSERT_EQ(i, value_b);
}

