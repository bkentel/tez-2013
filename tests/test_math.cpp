#include "pch.hpp"

#include <gtest/gtest.h>
#include "math.hpp"

TEST(MathTest, Point2dSanity) {
    using point = bklib::point2d<int>;

    int const x[] = {10, 11};
    int const y[] = {20, 21};

    //constructor test
    point const p {x[0], y[0]};
    ASSERT_EQ(p.x, x[0]);
    ASSERT_EQ(p.y, y[0]);
    ASSERT_EQ(p, p);

    //constructor test
    point const q {x[1], y[1]};
    ASSERT_EQ(q.x, x[1]);
    ASSERT_EQ(q.y, y[1]);
    ASSERT_EQ(q, q);

    //intersection test
    auto const pi = bklib::intersect(p, q);
    ASSERT_EQ(pi.first, false);

    //distance test
    ASSERT_EQ(bklib::distance(p, p),  0);
    ASSERT_EQ(bklib::distance2(p, p), 0);

    ASSERT_EQ(bklib::distance(q, q),  0);
    ASSERT_EQ(bklib::distance2(q, q), 0);

    auto const d  = bklib::distance(p, q);
    auto const d2 = bklib::distance2(p, q);
    ASSERT_FLOAT_EQ(d2, d*d);

    //reorder test
    ASSERT_EQ(bklib::reorder_x(p, q).first, p);
    ASSERT_EQ(bklib::reorder_x(q, p).first, p);

    ASSERT_EQ(bklib::reorder_x(p, q).second, q);
    ASSERT_EQ(bklib::reorder_x(q, p).second, q);

    ASSERT_EQ(bklib::reorder_y(p, q).first, p);
    ASSERT_EQ(bklib::reorder_y(q, p).first, p);

    ASSERT_EQ(bklib::reorder_y(p, q).second, q);
    ASSERT_EQ(bklib::reorder_y(q, p).second, q);

    //translate
    point const p0 = {0, 0};
    auto const p1 = bklib::translate(p0, 10, -10);
    ASSERT_EQ(p1.x, 10);
    ASSERT_EQ(p1.y, -10);
}

TEST(MathTest, AARectSanity) {
    using rect  = bklib::axis_aligned_rect<int>;
    using point = bklib::point2d<int>;

    int const x[] = {10, 20, 15, 25};
    int const y[] = {15, 25, 20, 30};

    rect r01 = {x[0], y[0], x[1], y[1]};
    auto r02 = rect(x[0], y[0], x[1], y[1]);
    auto r03 = rect(point{x[0], y[0]}, x[1] - x[0], y[1] - y[0]);

    ASSERT_EQ(r01.left(),   x[0]);
    ASSERT_EQ(r01.right(),  x[1]);
    ASSERT_EQ(r01.top(),    y[0]);
    ASSERT_EQ(r01.bottom(), y[1]);

    ASSERT_EQ(r01, r02);
    ASSERT_EQ(r02, r03);

    auto const w = 10;
    auto const h = 20;
    auto const p = point {-5, -5};
    auto const r04 = rect(p, w, h);
    ASSERT_EQ(r04.width(), w);
    ASSERT_EQ(r04.height(), h);
    ASSERT_EQ(r04.left(), p.x);
    ASSERT_EQ(r04.top(), p.y);

    ASSERT_EQ(bklib::intersect(r04, p), bklib::intersect(p, r04));

    rect const r1 = {5, 10, 15, 20};
    auto const r2 = bklib::translate(r1, -5, -10);
    ASSERT_EQ(r1.width(), r2.width());
    ASSERT_EQ(r1.height(), r2.height());
    ASSERT_EQ(r2.left(), 0);
    ASSERT_EQ(r2.top(), 0);
    ASSERT_EQ(r2.bottom(), 10);
    ASSERT_EQ(r2.right(), 10);
}
