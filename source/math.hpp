#pragma once

namespace bklib {
//==============================================================================
template <typename T>
struct index2d {
    static_assert(std::is_integral<T>::value, "indicies must be integral.");

    size_t as_size_t(size_t const stride, T dx = 0, T dy = 0) const BK_NOEXCEPT {
        BK_ASSERT(y + dy >= 0);
        BK_ASSERT(x + dx >= 0);

        auto const yy = static_cast<size_t(y + dy);
        auto const xx = static_cast<size_t(x + dx);

        return yy * stride + xx;
    }

    T x, y;
};
//==============================================================================
template <typename T> index2d<T> north_of(index2d<T> i) { return {i.x + 0, i.y - 1}; }
template <typename T> index2d<T> south_of(index2d<T> i) { return {i.x + 0, i.y + 1}; }
template <typename T> index2d<T> east_of(index2d<T> i)  { return {i.x + 1, i.y + 0}; }
template <typename T> index2d<T> west_of(index2d<T> i)  { return {i.x - 1, i.y + 0}; }

//==============================================================================
template <typename T>
struct point2d {
    T x, y;
};

template <typename T>
bool operator==(point2d<T> p, point2d<T> q) {
    static_assert(std::is_integral<T>::value, "not implemented"); //TODO
    return (p.x == q.x) && (p.y == q.y);
}

template <typename T>
std::pair<point2d<T>, point2d<T>>
reorder_x(point2d<T> p, point2d<T> q) {
    return (p.x <= q.x) ? std::make_pair(p, q) : std::make_pair(q, p);
}

template <typename T>
std::pair<point2d<T>, point2d<T>>
reorder_y(point2d<T> p, point2d<T> q) {
    return (p.y <= q.y) ? std::make_pair(p, q) : std::make_pair(q, p);
}

template <typename T>
T distance2(point2d<T> p, point2d<T> q) {
    auto const dx = (q.x - p.x);
    auto const dy = (q.y - p.y);

    return (dx * dx) + (dy * dy);
}

template <typename T, typename R = float>
R distance(point2d<T> p, point2d<T> q) {
    auto const dx = (q.x - p.x);
    auto const dy = (q.y - p.y);

    return std::sqrt(static_cast<R>((dx * dx) + (dy * dy)));
}

template <typename T>
std::pair<bool, point2d<T>>
intersect(point2d<T> p, point2d<T> q) {
    return (p == q) ? std::make_pair(true, p) : std::make_pair(false, p);
}

template <typename T, typename U, typename V>
point2d<T> translate(point2d<T> const p, U const dx, V const dy) {
    return {p.x + dx, p.y + dy};
}
//==============================================================================
template <typename T>
class axis_aligned_rect {
private:
    struct tag_unchecked {};
public:
    using value = T;
    using point = point2d<T>;

    axis_aligned_rect()
        : axis_aligned_rect(tag_unchecked{}, 0, 0, 0, 0)
    {
    }

    axis_aligned_rect(value x0, value y0, value x1, value y1)
        : x0_ {x0}, y0_ {y0}
        , x1_ {x1}, y1_ {y1}
    {
        BK_ASSERT(x1 > x0);
        BK_ASSERT(y1 > y0);
    }

    axis_aligned_rect(std::initializer_list<value> list)
        : axis_aligned_rect(
            *(list.begin() + 0)
          , *(list.begin() + 1)
          , *(list.begin() + 2)
          , *(list.begin() + 3)
        )
    {
        BK_ASSERT(list.size() == 4); 
    }

    axis_aligned_rect(point p, value w, value h)
        : axis_aligned_rect(p.x, p.y, p.x + w, p.y + h)
    {
        BK_ASSERT(w > 0);
        BK_ASSERT(h > 0);
    }

    void move_to_top_left(point p) BK_NOEXCEPT {
        auto const dx = p.x - x0_;
        auto const dy = p.y - y0_;

        x0_ += dx; x1_ += dx;
        y0_ += dy; y1_ += dy;
    }

    T area() const BK_NOEXCEPT { return width()* height(); }

    T left()   const BK_NOEXCEPT { return x0_; }
    T right()  const BK_NOEXCEPT { return x1_; }
    T top()    const BK_NOEXCEPT { return y0_; }
    T bottom() const BK_NOEXCEPT { return y1_; }

    point top_left()     const BK_NOEXCEPT { return {left(),  top()}; }
    point top_right()    const BK_NOEXCEPT { return {right(), top()}; }
    point bottom_left()  const BK_NOEXCEPT { return {left(),  bottom()}; }
    point bottom_right() const BK_NOEXCEPT { return {right(), bottom()}; }

    T width()  const BK_NOEXCEPT { return right()  - left(); }
    T height() const BK_NOEXCEPT { return bottom() - top();  }

    std::pair<bool, axis_aligned_rect<T>> intersect(axis_aligned_rect<T> const other) const BK_NOEXCEPT {
        auto const& ra = *this;
        auto const& rb = other;

        auto const l = std::max(ra.left(),   rb.left());
        auto const r = std::min(ra.right(),  rb.right());
        auto const t = std::max(ra.top(),    rb.top());
        auto const b = std::min(ra.bottom(), rb.bottom());

        return {(l < r) && (t < b), {tag_unchecked{}, l, t, r, b}};
    }
private:
    axis_aligned_rect(tag_unchecked, value x0, value y0, value x1, value y1) BK_NOEXCEPT
        : x0_ {x0}, y0_ {y0}
        , x1_ {x1}, y1_ {y1}
    {
    }    

    bool is_well_formed() const BK_NOEXCEPT {
        return (left() < right()) && (top() < bottom());
    }

    T x0_, x1_;
    T y0_, y1_;
};

template <typename T>
bool operator==(axis_aligned_rect<T> p, axis_aligned_rect<T> q) {
    static_assert(std::is_integral<T>::value, "not implemented"); //TODO

    return p.left()   == q.left()
        && p.right()  == q.right()
        && p.top()    == q.top()
        && p.bottom() == q.bottom();
}

template <typename T>
std::pair<bool, axis_aligned_rect<T>>
intersect(axis_aligned_rect<T> const ra, axis_aligned_rect<T> const rb) {
    return ra.intersect(rb);
}

template <typename T, typename U, typename V>
axis_aligned_rect<T> translate(axis_aligned_rect<T> const r, U const dx, V const dy) {
    return axis_aligned_rect<T>(translate(r.top_left(), dx, dy), r.width(), r.height());
}

//==============================================================================
template <typename T>
std::pair<bool, point2d<T>>
intersect(axis_aligned_rect<T> const r, point2d<T> const p) {
    return {
        p.x >= r.left() && p.x < r.right() && p.y >= r.top() && p.y < r.bottom()
      , p
    };
}

template <typename T>
std::pair<bool, point2d<T>>
intersect(point2d<T> const p, axis_aligned_rect<T> const r) {
    return intersect(r, p);
}

} //namespace bklib
