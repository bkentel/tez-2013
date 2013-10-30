#include "pch.hpp"

#include <gtest/gtest.h>
#include "math.hpp"

//==============================================================================
// Basic tests for points and vectors and their interaction.
//==============================================================================
TEST(Math, PointAndVector) {
    using namespace bklib;

    point2d<int> const p1 {10, 10};
    point2d<int> const p2 {15, 16};

    ASSERT_EQ((p1 - p2), -(p2 - p1));
    ASSERT_EQ(dot(p1 - p2), dot(p2 - p1));
    ASSERT_EQ(p1 + (p2 - p1), p2);
    ASSERT_EQ(p2 + (p1 - p2), p1);

    ASSERT_EQ(magnitude(p1 - p2), magnitude(p2 - p1));
    ASSERT_EQ(direction<float>(p1 - p2), -direction<float>(p2 - p1));

    ASSERT_FLOAT_EQ(1.0f, magnitude(direction<float>(p1 - p2)));
    ASSERT_FLOAT_EQ(1.0f, magnitude(direction<float>(p2 - p1)));
}

//==============================================================================
// Sanity tests for rectangles.
//==============================================================================
TEST(Math, AARectIntersectionsSimple) {
    using namespace bklib;
    
    auto const x = 3;
    auto const y = 5;
    auto const w = 6;
    auto const h = 8;
    
    auto const tl = point2d<int>{x, y};
    auto const tr = point2d<int>{x + w, y};
    auto const bl = point2d<int>{x, y + h};
    auto const br = point2d<int>{x + w, y + h};

    using rect = axis_aligned_rect<int>;
    auto const r = rect{tl, w, h};

    ASSERT_TRUE(r.is_well_formed());

    ASSERT_EQ(r.left(),   tl.x);
    ASSERT_EQ(r.top(),    tl.y);
    ASSERT_EQ(r.right(),  br.x);
    ASSERT_EQ(r.bottom(), br.y);

    ASSERT_EQ(tl, r.get_point(rect::top_left_t{}));
    ASSERT_EQ(tr, r.get_point(rect::top_right_t{}));
    ASSERT_EQ(bl, r.get_point(rect::bot_left_t{}));
    ASSERT_EQ(br, r.get_point(rect::bot_right_t{}));

    ASSERT_EQ(r.width(),  w);
    ASSERT_EQ(r.height(), h);

    ASSERT_EQ(w * h, area(r));
}

//==============================================================================
// Tests for valid intersections of rects.
//==============================================================================
TEST(Math, AARectIntersectionsValid) {
    using namespace bklib;

    using point = point2d<int>;

    axis_aligned_rect<int> const ra = {point2d<int>{0, 0}, 5, 5};

    axis_aligned_rect<int> const rb[] = {
        {point2d<int>{-1, -1}, 3, 3},
        {point2d<int>{ 1, -1}, 3, 3},
        {point2d<int>{ 3, -1}, 3, 3},
        {point2d<int>{-1,  1}, 3, 3},
        {point2d<int>{ 1,  1}, 3, 3},
        {point2d<int>{ 3,  1}, 3, 3},
        {point2d<int>{-1,  3}, 3, 3},
        {point2d<int>{ 1,  3}, 3, 3},
        {point2d<int>{ 3,  3}, 3, 3},
    };

    int const areas[] = {4, 6, 4, 6, 9, 6, 4, 6, 4};

    axis_aligned_rect<int> const rc[] = {
        {point{0, 0}, 2, 2},
        {point{1, 0}, 3, 2},
        {point{3, 0}, 2, 2},
        {point{0, 1}, 2, 3},
        {point{1, 1}, 3, 3},
        {point{3, 1}, 2, 3},
        {point{0, 3}, 2, 2},
        {point{1, 3}, 3, 2},
        {point{3, 3}, 2, 2},
    };

    for (int i = 0; i < 9; ++i) {
        ASSERT_TRUE(intersects(ra, rb[i]));
        auto const ir = intersection_of(ra, rb[i]);
        ASSERT_FALSE(!ir);
        ASSERT_TRUE(ir.result.is_well_formed());
        ASSERT_EQ(areas[i], area(ir.result));
        ASSERT_EQ(rc[i], ir.result);
    }
}

//==============================================================================
// Tests for invalid intersections of rects.
//==============================================================================
TEST(Math, AARectIntersectionsInvalid) {
    using namespace bklib;

    axis_aligned_rect<int> const r  = {point2d<int>{0, 0}, 10, 10};
    axis_aligned_rect<int> const r1 = {10, 1,  11, 11};
    axis_aligned_rect<int> const r2 = {1,  10, 11, 11};
    axis_aligned_rect<int> const r3 = {10, 10, 11, 11};

    ASSERT_FALSE(!!intersection_of(r, r1));
    ASSERT_FALSE(!!intersection_of(r, r2));
    ASSERT_FALSE(!!intersection_of(r, r3));

    ASSERT_FALSE(intersection_of(r, r1).result.is_well_formed());
    ASSERT_FALSE(intersection_of(r, r2).result.is_well_formed());
    ASSERT_FALSE(intersection_of(r, r3).result.is_well_formed());

    ASSERT_FALSE(intersects(r, r1));
    ASSERT_FALSE(intersects(r, r2));
    ASSERT_FALSE(intersects(r, r3));
}
