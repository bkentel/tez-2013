#include "pch.hpp"

#include <gtest/gtest.h>
#include "math.hpp"

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

TEST(Math, AARectIntersectionsTrue) {
    using namespace bklib;

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
        {0, 0, 1, 1},
        {1, 0, 3, 1},
        {3, 0, 5, 1},
        {0, 1, 1, 3},
        {1, 1, 3, 3},
        {3, 1, 5, 3},
        {0, 3, 1, 5},
        {1, 3, 3, 5},
        {3, 3, 5, 5},
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

TEST(Math, AARectIntersectionsFalse) {
    using namespace bklib;

    axis_aligned_rect<int> const r  = {0, 0, 9, 9};
    axis_aligned_rect<int> const r1 = {9, 1, 10, 10};
    axis_aligned_rect<int> const r2 = {1, 9, 10, 10};
    axis_aligned_rect<int> const r3 = {9, 9, 10, 10};

    ASSERT_TRUE(!intersection_of(r, r1));
    ASSERT_TRUE(!intersection_of(r, r2));
    ASSERT_TRUE(!intersection_of(r, r3));

    ASSERT_FALSE(intersection_of(r, r1).result.is_well_formed());
    ASSERT_FALSE(intersection_of(r, r2).result.is_well_formed());
    ASSERT_FALSE(intersection_of(r, r3).result.is_well_formed());

    ASSERT_FALSE(intersects(r, r1));
    ASSERT_FALSE(intersects(r, r2));
    ASSERT_FALSE(intersects(r, r3));
}

TEST(MathTest, Point2dSanity) {
    //using point = bklib::point2d<int>;

    //int const x[] = {10, 11};
    //int const y[] = {20, 21};

    ////constructor test
    //point const p {x[0], y[0]};
    //ASSERT_EQ(p.x, x[0]);
    //ASSERT_EQ(p.y, y[0]);
    //ASSERT_EQ(p, p);

    ////constructor test
    //point const q {x[1], y[1]};
    //ASSERT_EQ(q.x, x[1]);
    //ASSERT_EQ(q.y, y[1]);
    //ASSERT_EQ(q, q);

    ////intersection test
    //auto const pi = bklib::intersect(p, q);
    //ASSERT_EQ(pi.first, false);

    ////distance test
    //ASSERT_EQ(bklib::distance(p, p),  0);
    //ASSERT_EQ(bklib::distance2(p, p), 0);

    //ASSERT_EQ(bklib::distance(q, q),  0);
    //ASSERT_EQ(bklib::distance2(q, q), 0);

    //auto const d  = bklib::distance(p, q);
    //auto const d2 = bklib::distance2(p, q);
    //ASSERT_FLOAT_EQ(d2, d*d);

    ////reorder test
    //ASSERT_EQ(bklib::reorder_x(p, q).first, p);
    //ASSERT_EQ(bklib::reorder_x(q, p).first, p);

    //ASSERT_EQ(bklib::reorder_x(p, q).second, q);
    //ASSERT_EQ(bklib::reorder_x(q, p).second, q);

    //ASSERT_EQ(bklib::reorder_y(p, q).first, p);
    //ASSERT_EQ(bklib::reorder_y(q, p).first, p);

    //ASSERT_EQ(bklib::reorder_y(p, q).second, q);
    //ASSERT_EQ(bklib::reorder_y(q, p).second, q);

    ////translate
    //point const p0 = {0, 0};
    //auto const p1 = bklib::translate(p0, 10, -10);
    //ASSERT_EQ(p1.x, 10);
    //ASSERT_EQ(p1.y, -10);
}

TEST(MathTest, AARectSanity) {
    //using rect  = bklib::axis_aligned_rect<int>;
    //using point = bklib::point2d<int>;

    //int const x[] = {10, 20, 15, 25};
    //int const y[] = {15, 25, 20, 30};

    //rect r01 = {x[0], y[0], x[1], y[1]};
    //auto r02 = rect(x[0], y[0], x[1], y[1]);
    //auto r03 = rect(point{x[0], y[0]}, x[1] - x[0], y[1] - y[0]);

    //ASSERT_EQ(r01.left(),   x[0]);
    //ASSERT_EQ(r01.right(),  x[1]);
    //ASSERT_EQ(r01.top(),    y[0]);
    //ASSERT_EQ(r01.bottom(), y[1]);

    //ASSERT_EQ(r01, r02);
    //ASSERT_EQ(r02, r03);

    //auto const w = 10;
    //auto const h = 20;
    //auto const p = point {-5, -5};
    //auto const r04 = rect(p, w, h);
    //ASSERT_EQ(r04.width(), w);
    //ASSERT_EQ(r04.height(), h);
    //ASSERT_EQ(r04.left(), p.x);
    //ASSERT_EQ(r04.top(), p.y);

    //ASSERT_EQ(bklib::intersect(r04, p), bklib::intersect(p, r04));

    //rect const r1 = {5, 10, 15, 20};
    //auto const r2 = bklib::translate(r1, -5, -10);
    //ASSERT_EQ(r1.width(), r2.width());
    //ASSERT_EQ(r1.height(), r2.height());
    //ASSERT_EQ(r2.left(), 0);
    //ASSERT_EQ(r2.top(), 0);
    //ASSERT_EQ(r2.bottom(), 10);
    //ASSERT_EQ(r2.right(), 10);
}
