//!=============================================================================
//! @file
//! @author Brandon Kentel
//! @brief File containing example of doxygen usage for quick reference.
//!=============================================================================
#pragma once

namespace bklib {

struct point_tl {};
struct point_tr {};
struct point_bl {};
struct point_br {};
struct point_center {};

struct side_top {};
struct side_bottom {};
struct side_left {};
struct side_right {};

template <typename Test, typename Type = void>
using enable_for_floating_point_t = typename std::enable_if<
    std::is_floating_point<Test>::value, Type
>::type;

template <typename Test, typename Type = void>
using enable_for_integral_t = typename std::enable_if<
    std::is_integral<Test>::value, Type
>::type;

//MOVE
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
//MOVE


//!=============================================================================
//! @see if_not_void_t.
//!=============================================================================
template <typename Test, typename Default>
struct if_not_void : std::conditional<
    std::is_void<Test>::value, Default, Test
>
{
};

template <typename Test, typename T1, typename T2>
struct if_not_void_common : std::conditional<
    std::is_void<Test>::value, typename std::common_type<T1, T2>::type, Test
>
{
};

//!=============================================================================
//! If @c Test is @c void, @c Default, otherwise @c Test.
//!=============================================================================
template <typename Test, typename Default>
using if_not_void_t = typename if_not_void<Test, Default>::type;

template <typename Test, typename T1, typename T2>
using if_not_void_common_t = typename if_not_void_common<Test, T1, T2>::type;


//!=============================================================================
//! Return (-1, 0, 1) corresponding to the sign of @c x.
//!=============================================================================
template <typename T, enable_for_integral_t<T>* = 0>
T sign_of(T const x) {
    return (x > T{0}) - (x < T{0});
}

template <typename T, enable_for_floating_point_t<T>* = 0>
T sign_of(T const x) {
    return 0; //TODO
}

//!=============================================================================
//! Unified template for fixed size integer types.
//!=============================================================================
template <unsigned N, bool Signed = true>
struct sized_integral;

template <> struct sized_integral<1, true> { using type = std::int8_t;  };
template <> struct sized_integral<2, true> { using type = std::int16_t; };
template <> struct sized_integral<4, true> { using type = std::int32_t; };
template <> struct sized_integral<8, true> { using type = std::int64_t; };

template <> struct sized_integral<1, false> { using type = std::uint8_t;  };
template <> struct sized_integral<2, false> { using type = std::uint16_t; };
template <> struct sized_integral<4, false> { using type = std::uint32_t; };
template <> struct sized_integral<8, false> { using type = std::uint64_t; };

template <unsigned N>
using sized_signed_t = typename sized_integral<N, true>::type;

template <unsigned N>
using sized_unsigned_t = typename sized_integral<N, false>::type;
//!=============================================================================
//! Equality comparison for floating point, otherwise use built in comparison.
//!=============================================================================
template <typename T, enable_for_floating_point_t<T>* = 0>
bool is_equal(T const a, T const b, int const ulps = 4) {
    BK_ASSERT(ulps > 0);

    using int_t = sized_signed_t<sizeof(T)>;

    static auto const max = std::numeric_limits<int_t>::max() + 1;

    auto const ai = *reinterpret_cast<int_t const*>(&a);
    auto const bi = *reinterpret_cast<int_t const*>(&b);

    auto const an = ai < 0 ? max - ai : ai;
    auto const bn = bi < 0 ? max - bi : bi;

    return std::abs(an - bn) <= ulps;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
bool is_equal(T const a, U const b) {
    return a == b;
}

template <typename T>
T square_of(T const x) { return x*x; }

//!=============================================================================
//! 2D point type.
//!=============================================================================
template <typename T>
struct point2d { T x, y; };
//!=============================================================================
//! 2D vector type.
//!=============================================================================
template <typename T>
struct vector2d { T x, y; };
//!=============================================================================
//! Circle type.
//!=============================================================================
template <typename T>
struct circle {
    point2d<T> p;
    T r;
};

namespace detail {
    struct axis_aligned_rect_base {
        struct allow_malformed {};

        template <typename T, typename Tag>
        struct tagged_point {
            tagged_point(T x, T y) : value{{x, y}} {}
            tagged_point(point2d<T> p) : value{{p.x, p.y}} {}
            point2d<T> value;
        };

        struct tag_tl_point;
        struct tag_tr_point;
        struct tag_bl_point;
        struct tag_br_point;
        struct tag_center_point;
    };
} //namespace detail

//!=============================================================================
//! Axis-aligned rectangle type.
//!=============================================================================
template <typename T>
class axis_aligned_rect : public detail::axis_aligned_rect_base {
public:
    using value  = T;
    using point  = point2d<T>;
    using vector = vector2d<T>;

    axis_aligned_rect(allow_malformed, T x0, T y0, T x1, T y1) BK_NOEXCEPT
      : x0_{x0}, y0_{y0}, x1_{x1}, y1_{y1}
    {
    }

    axis_aligned_rect(T x0, T y0, T x1, T y1) BK_NOEXCEPT
      : axis_aligned_rect(allow_malformed{}, x0, y0, x1, y1)
    {
        BK_ASSERT(x1 > x0);
        BK_ASSERT(y1 > y0);
    }

    axis_aligned_rect() BK_NOEXCEPT
      : axis_aligned_rect(allow_malformed{}, T{0}, T{0}, T{0}, T{0})
    {
    }

    axis_aligned_rect(std::initializer_list<value> list) BK_NOEXCEPT
        : axis_aligned_rect(
            *(list.begin() + 0)
          , *(list.begin() + 1)
          , *(list.begin() + 2)
          , *(list.begin() + 3)
        )
    {
        BK_ASSERT(list.size() == 4);
    }

    using tl_point     = tagged_point<T, tag_tl_point>;
    using tr_point     = tagged_point<T, tag_tr_point>;
    using bl_point     = tagged_point<T, tag_bl_point>;
    using br_point     = tagged_point<T, tag_br_point>;
    using center_point = tagged_point<T, tag_center_point>;

    axis_aligned_rect(tl_point p, value w, value h) BK_NOEXCEPT
      : axis_aligned_rect(p.value, w, h)
    {
        BK_ASSERT(w > 0);
        BK_ASSERT(h > 0);
    }

    axis_aligned_rect(center_point p, value w, value h) BK_NOEXCEPT
      : axis_aligned_rect(p.value - [&]()->vector2d<T> { return {w / T{2}, h / T{2}}; }(), w, h)
    {
    }

    axis_aligned_rect& translate(vector const v) {
        x0_ += v.x; x1_ += v.x;
        y0_ += v.y; y1_ += v.y;

        return *this;
    }

    T width()  const BK_NOEXCEPT { return right()  - left(); }
    T height() const BK_NOEXCEPT { return bottom() - top();  }

    T left()   const BK_NOEXCEPT { return x0_; }
    T right()  const BK_NOEXCEPT { return x1_; }
    T top()    const BK_NOEXCEPT { return y0_; }
    T bottom() const BK_NOEXCEPT { return y1_; }

    point top_left()     const BK_NOEXCEPT { return {left(),  top()}; }
    point top_right()    const BK_NOEXCEPT { return {right(), top()}; }
    point bottom_left()  const BK_NOEXCEPT { return {left(),  bottom()}; }
    point bottom_right() const BK_NOEXCEPT { return {right(), bottom()}; }

    template <typename R = T>
    point2d<R> center() const BK_NOEXCEPT {
        R const x = left() + width()  / R{2};
        R const y = top()  + height() / R{2};
        return {x, y};
    }

    bool is_well_formed() const BK_NOEXCEPT {
        return (left() < right()) && (top() < bottom());
    }
private:
    axis_aligned_rect(point p, value w, value h) BK_NOEXCEPT
        : axis_aligned_rect(p.x, p.y, p.x + w, p.y + h)
    {
    }

    T x0_, x1_;
    T y0_, y1_;
};
//!=============================================================================
//! Result type for geometric intersections.
//!=============================================================================
template <typename T>
struct intersection_result {
    bool valid;
    T    result;

    explicit operator bool() const BK_NOEXCEPT { return valid; }
};
//!=============================================================================
//! The geometric union of n objects.
//!=============================================================================
template <typename T, typename... Types>
struct geometric_union {
    using point = point2d<T>;
    
    //Recursive case
    template <int I = -1>
    bool intersects(point const p) const {
        return bklib::intersects(std::get<I-1>(types), p) && intersects<I-2>(p);
    }

    //Default case
    template <>
    bool intersects<-1>(point const p) const {
        return intersects<sizeof...(Types)>(p);
    }

    //Base case
    template <>
    bool intersects<0>(point const p) const {
        return bklib::intersects(std::get<0>(types), p);
    }

    std::tuple<Types...> types;
};
//------------------------------------------------------------------------------
// Base case
//------------------------------------------------------------------------------
template <typename T>
struct geometric_union<T> {
    using point = point2d<T>;

    template <int I = -1>
    bool intersects(point const p) const {
        return true;
    }
};

template <typename T, typename... Types>
intersection_result<point2d<T>> intersects(
    geometric_union<Types...> const u,
    point2d<T> const p
) {
    return {u.intersects(p), p};
}
//==============================================================================
// Global built-in operators.
//==============================================================================
template <typename T, typename U>
using common_type_t = typename std::common_type<T, U>::type;
//------------------------------------------------------------------------------
template <typename T>
bool operator==(
    axis_aligned_rect<T> const lhs
  , axis_aligned_rect<T> const rhs
) BK_NOEXCEPT {
    return is_equal(lhs.left(),   rhs.left())
        && is_equal(lhs.right(),  rhs.right())
        && is_equal(lhs.top(),    rhs.top())
        && is_equal(lhs.bottom(), rhs.bottom());
}
//------------------------------------------------------------------------------
template <typename T>
bool operator!=(
    axis_aligned_rect<T> const lhs
  , axis_aligned_rect<T> const rhs
) BK_NOEXCEPT {
    return !(lhs == rhs);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator==(point2d<T> const lhs, point2d<T> const rhs) BK_NOEXCEPT {
    return is_equal(lhs.x, rhs.x)
        && is_equal(lhs.y, rhs.y);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator!=(point2d<T> const lhs, point2d<T> const rhs) BK_NOEXCEPT {
    return !(lhs == rhs);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator==(vector2d<T> const lhs, vector2d<T> const rhs) BK_NOEXCEPT {
    return is_equal(lhs.x, rhs.x)
        && is_equal(lhs.y, rhs.y);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator!=(vector2d<T> const lhs, vector2d<T> const rhs) BK_NOEXCEPT {
    return !(lhs == rhs);
}
//------------------------------------------------------------------------------
template <typename T>
auto operator-(vector2d<T> const v) BK_NOEXCEPT
-> vector2d<T> {
    return {-v.x, -v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator*(vector2d<T> const v, U const c) BK_NOEXCEPT
-> vector2d<common_type_t<T, U>> {
    return {c * v.x, c * v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator*(T const c, vector2d<U> const v) BK_NOEXCEPT
-> vector2d<common_type_t<T, U>> {
    return v * c;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator/(vector2d<T> const v, U const c) BK_NOEXCEPT
-> vector2d<common_type_t<T, U>> {
    return {v.x / c, v.y / c};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator/(U const c, vector2d<T> const v) BK_NOEXCEPT
-> vector2d<common_type_t<T, U>> {
    return v / c;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator+(vector2d<T> const u, vector2d<U> const v) BK_NOEXCEPT
-> vector2d<common_type_t<T, U>> {
    return {u.x + v.x, u.y + v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator+(point2d<T> const p, vector2d<U> const v) BK_NOEXCEPT
-> point2d<common_type_t<T, U>> {
    return {p.x + v.x, p.y + v.y};
}
//------------------------------------------------------------------------------
template <typename T>
auto operator+(axis_aligned_rect<T> const r, vector2d<T> const v) BK_NOEXCEPT
-> axis_aligned_rect<T> {
    auto const p = r.top_left();
    auto const w = r.width();
    auto const h = r.height();

    return {axis_aligned_rect<T>::tl_point(p + v), w, h};
}
//------------------------------------------------------------------------------
template <typename T>
auto operator-(vector2d<T> const u, vector2d<T> const v) BK_NOEXCEPT
-> vector2d<T> {
    return {u.x - v.x, u.y - v.y};
}
//------------------------------------------------------------------------------
template <typename T>
auto operator-(point2d<T> const p, vector2d<T> const v) BK_NOEXCEPT
-> point2d<T> {
    return {p.x - v.x, p.y - v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator-(point2d<T> const p, point2d<U> const q) BK_NOEXCEPT
-> vector2d<common_type_t<T, U>> {
    return {p.x - q.x, p.y - q.y};
}
//------------------------------------------------------------------------------
template <typename T, typename R>
auto operator*=(vector2d<T>& lhs, vector2d<T> const rhs) BK_NOEXCEPT
-> vector2d<T>& {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}
//------------------------------------------------------------------------------
template <typename T>
auto operator+=(vector2d<T>& lhs, vector2d<T> const rhs) BK_NOEXCEPT
-> vector2d<T>& {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}
//------------------------------------------------------------------------------
template <typename T>
auto operator+=(point2d<T>& lhs, vector2d<T> const rhs) BK_NOEXCEPT
-> point2d<T>& {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}
//------------------------------------------------------------------------------
template <typename T>
auto operator-=(vector2d<T>& lhs, vector2d<T> const rhs) BK_NOEXCEPT
-> vector2d<T>& {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}
//------------------------------------------------------------------------------
template <typename T>
auto operator-=(point2d<T>& lhs, vector2d<T> const rhs) BK_NOEXCEPT
-> point2d<T>& {
    lhs.x -= rhs.x;
    lhs.y -= rhs.y;
    return lhs;
}
//==============================================================================
// Other operators.
//==============================================================================
template <typename R = void, typename T = void, typename U = void>
auto dot(vector2d<T> const u, vector2d<U> const v) BK_NOEXCEPT
-> if_not_void_common_t<R, T, U> {
    return if_not_void_common_t<R, T, U> {u.x * v.x + u.y * v.y};
}
//------------------------------------------------------------------------------
template <typename R = void, typename T = void>
auto dot(vector2d<T> const v) BK_NOEXCEPT
-> if_not_void_t<R, T> {
    using type = if_not_void_t<R, T>;
    return static_cast<type>(dot(v, v));
}
//------------------------------------------------------------------------------
template <typename R = void, typename T = void>
auto magnitude(vector2d<T> const v) BK_NOEXCEPT
-> if_not_void_t<R, T> {
    using result = if_not_void_t<R, T>;
    auto const mag2 = static_cast<result>(dot(v, v));
    return static_cast<result>(std::sqrt(mag2));
}
//------------------------------------------------------------------------------
template <typename R = void, typename T = void>
auto direction(vector2d<T> const v) BK_NOEXCEPT
-> vector2d<if_not_void_t<R, T>> {
    using result = if_not_void_t<R, T>;
    auto const mag = magnitude<result>(v);

    if (is_equal(mag, result{0})) {
        return {result{0}, result{0}};
    }

    return v / mag;
}
//------------------------------------------------------------------------------
template <typename T>
T area(axis_aligned_rect<T> const r) BK_NOEXCEPT {
    return r.width() * r.height();
}
//==============================================================================
// Conversions.
//==============================================================================
template <typename R, typename T>
auto to_type(vector2d<T> const v) BK_NOEXCEPT
-> vector2d<R> {
    return {static_cast<R>(v.x), static_cast<R>(v.y)};
}

template <typename R, typename T>
auto to_type(point2d<T> const p) BK_NOEXCEPT
-> point2d<R> {
    return {static_cast<R>(p.x), static_cast<R>(p.y)};
}

//==============================================================================
// Distances
//==============================================================================
//------------------------------------------------------------------------------
// point <-> point
//------------------------------------------------------------------------------
template <typename T, typename U>
auto distance2(point2d<T> const a, point2d<U> const b) BK_NOEXCEPT
-> common_type_t<T, U> {
    return dot(a - b);
}
//------------------------------------------------------------------------------
template <typename R = void, typename T = void, typename U = void>
auto distance(point2d<T> const a, point2d<U> const b) BK_NOEXCEPT
-> if_not_void_t<R, common_type_t<T, U>> {
    using result_t = if_not_void_t<R, common_type_t<T, U>>;

    return static_cast<result_t>(
        std::sqrt(distance2(a, b))
    );
}
//------------------------------------------------------------------------------
// circle <-> circle
//------------------------------------------------------------------------------
template <typename R = void, typename T = void, typename U = void>
auto distance(circle<T> const a, circle<U> const b) BK_NOEXCEPT
-> if_not_void_t<R, common_type_t<T, U>> {
    using result_t = if_not_void_t<R, T>;

    return static_cast<result_t>(
        std::sqrt(dot(a.p - b.p)) - a.r - b.r
    );
}
//------------------------------------------------------------------------------
// rectangle <-> rectangle
//------------------------------------------------------------------------------
template <
    typename CornerA = axis_aligned_rect<T>::center_t
  , typename CornerB = axis_aligned_rect<T>::center_t
  , typename T = void
>
auto distance2(
    axis_aligned_rect<T> const ra
  , axis_aligned_rect<T> const rb
  , CornerA = CornerA{}
  , CornerB = CornerB{}
) BK_NOEXCEPT
-> T {
    auto const a = ra.get_point(CornerA{});
    auto const b = ra.get_point(CornerB{});

    return dot(a - b);
}
//==============================================================================
// Intersections
//==============================================================================
//------------------------------------------------------------------------------
// point <-> circle intersection.
//------------------------------------------------------------------------------
template <typename T>
auto intersects(circle<T> const c, point2d<T> const p) BK_NOEXCEPT
-> bool {
    return distance2(p, c.p) < square_of(c.r);
}

template <typename T>
auto intersects(point2d<T> const p, circle<T> const c) BK_NOEXCEPT
-> bool {
    return intersects(c, p);
}

//------------------------------------------------------------------------------
// axis_aligned_rect <-> circle intersection.
//------------------------------------------------------------------------------
template <typename T>
auto intersects(axis_aligned_rect<T> const r, circle<T> const c) BK_NOEXCEPT
-> bool {
    return intersects(bounding_circle<T>(r), c) && (
        intersects(c, r.top_left())
     || intersects(c, r.top_right())
     || intersects(c, r.bottom_left())
     || intersects(c, r.bottom_right())
    );
}

//------------------------------------------------------------------------------
// axis_aligned_rect <-> axis_aligned_rect intersection.
//------------------------------------------------------------------------------
template <typename T>
auto intersection_of(
    axis_aligned_rect<T> const ra
  , axis_aligned_rect<T> const rb
) BK_NOEXCEPT
-> intersection_result<axis_aligned_rect<T>> {
    auto const l = std::max(ra.left(),   rb.left());
    auto const r = std::min(ra.right(),  rb.right());
    auto const t = std::max(ra.top(),    rb.top());
    auto const b = std::min(ra.bottom(), rb.bottom());

    return {
        (l < r) && (t < b)
      , {axis_aligned_rect<T>::allow_malformed{}, l, t, r, b}
    };   
}
//------------------------------------------------------------------------------
template <typename T>
bool intersects(
    axis_aligned_rect<T> const ra
  , axis_aligned_rect<T> const rb
) BK_NOEXCEPT {
    return !(
        ra.right()  <= rb.left()
     || ra.bottom() <= rb.top()
     || ra.left()   >= rb.right()
     || ra.top()    >= rb.bottom()
    );
}
//------------------------------------------------------------------------------
// point <-> axis_aligned_rect
//------------------------------------------------------------------------------
template <typename T>
bool intersects(
    axis_aligned_rect<T> const r
  , point2d<T>           const p
) BK_NOEXCEPT {
    return !(
        p.x < r.left() || p.x >= r.right()
     || p.y < r.top()  || p.y >= r.bottom()
    );
}
//------------------------------------------------------------------------------
template <typename T>
bool intersects(
    point2d<T>           const p
  , axis_aligned_rect<T> const r
) BK_NOEXCEPT {
    return intersects(r, p);
}
//------------------------------------------------------------------------------
template <typename T>
auto intersection_of(
    axis_aligned_rect<T> const r
  , point2d<T>           const p
) BK_NOEXCEPT
-> intersection_result<point2d<T>> {
    return {intersects{r, p}, p};
}
//------------------------------------------------------------------------------
template <typename T>
auto intersection_of(  
    point2d<T>           const p
  , axis_aligned_rect<T> const r
) BK_NOEXCEPT
-> intersection_result<point2d<T>> {
    return intersection_of(r, p);
}
//------------------------------------------------------------------------------
template <typename T>
bool intersects(
    circle<T> const a
  , circle<T> const b
) BK_NOEXCEPT {
    auto const dist = distance2(a.p, b.p);
    auto const r    = square_of(a.r + b.r);

    return dist < r;
}

//!=============================================================================
//! Generate a random direction vector.
//!=============================================================================
template <typename T = float, typename F = void>
vector2d<T> random_direction(F& random) {
    static float const pi2 = 2.0f * std::acos(0.0f);

    auto const angle = std::uniform_real_distribution<float>(0.0f, pi2)(random);

    auto const x = static_cast<T>(std::sin(angle));
    auto const y = static_cast<T>(std::cos(angle));

    return {x, y};
}

//!=============================================================================
//! Floating -> Integral : round to next most negative / positive integer.
//! Floating -> Floating : type cast.
//! Integral -> Integral : type cast.
//! Integral -> Floating : type cast.
//!=============================================================================
template <typename Result, typename Source, 
    typename std::enable_if<
        std::is_integral<Result>::value
     && std::is_floating_point<Source>::value
    >::type* = 0
>
Result round_up_if_integral(Source const x) BK_NOEXCEPT {
    return (x >= Source{0})
      ? static_cast<Result>(std::ceil(x))
      : static_cast<Result>(std::floor(x));
}
//------------------------------------------------------------------------------
template <typename Result, typename Source,
    typename std::enable_if<!(
        std::is_integral<Result>::value
     && std::is_floating_point<Source>::value
    )>::type* = 0
>
Result round_up_if_integral(Source const x) BK_NOEXCEPT {
    return static_cast<Result>(x);
}

//!=============================================================================
//! Return the circle that rect can be inscribed in.
//!=============================================================================
template <typename R = float, typename T = void>
auto bounding_circle(axis_aligned_rect<T> const rect)
-> circle<R> {
    auto const p = rect.center<float>();
    auto const q = to_type<float>(rect.top_left());
    auto const r = round_up_if_integral<R>(distance(p, q));

    return {to_type<R>(p), r};
}

////////////////////////////////////////////////////////////////////////////////
// Translations
////////////////////////////////////////////////////////////////////////////////

//! Translate an axis_aligned_rect.
template <typename T>
auto translate(axis_aligned_rect<T> const r, vector2d<T> const v) BK_NOEXCEPT
-> axis_aligned_rect<T> {
    auto const p = axis_aligned_rect<T>::tl_point{r.top_left() + v};
    return {p, r.width(), r.height()};
}

//! Translate a circle.
template <typename T>
auto translate(circle<T> const c, vector2d<T> const v) BK_NOEXCEPT
-> circle<T> {
    return {c.p + v, c.r};
}

template <typename R, typename T, enable_for_floating_point_t<T>* = 0>
point2d<R> ceil(point2d<T> const p) {
    return {static_cast<R>(std::ceil(p.x)), static_cast<R>(std::ceil(p.y))};
}

template <typename R, typename T, enable_for_integral_t<T>* = 0>
point2d<R> ceil(point2d<T> const p) {
    return p;
}

template <typename R, typename T>
vector2d<R> round_toward(vector2d<T> const v) {
    static_assert(std::is_integral<R>::value, "must be integral");
    static_assert(std::is_floating_point<T>::value, "must be floating");

    auto const x = v.x >= T{0} ? std::ceil(v.x) : std::floor(v.x);
    auto const y = v.y >= T{0} ? std::ceil(v.y) : std::floor(v.y);

    return {static_cast<R>(x), static_cast<R>(y)};
}

template <typename R, typename T, typename U>
point2d<R> round_toward(point2d<T> const p, vector2d<U> const v) {
    static_assert(std::is_integral<R>::value, "must be integral");
    static_assert(std::is_floating_point<T>::value, "must be floating");

    auto const x = v.x >= T{0} ? std::ceil(p.x) : std::floor(p.x);
    auto const y = v.y >= T{0} ? std::ceil(p.y) : std::floor(p.y);

    return {static_cast<R>(x), static_cast<R>(y)};
}

//

//
////---------------
//
//
//
//
//
////------------------------------------------------------------------------------
//// circle <-> point intersection
////------------------------------------------------------------------------------
//template <typename T>
//intersection_result<point2d<T>>
//intersection_of(circle<T> const c, point2d<T> const p) {
//    auto const l2 = dot(c.p - p);
//    auto const r2 = c.r * c.r;
//
//    return {l2 < r2, p};
//}
//
//template <typename T>
//intersection_result<point2d<T>>
//intersection_of(point2d<T> const p, circle<T> const c) {
//    return intersection_of(c, p);
//}
//
////------------------------------------------------------------------------------
//// circle <-> circle intersection
////------------------------------------------------------------------------------
//template <typename T>
//intersection_result<
//    geometric_union<circle<T>, circle<T>>
//>
//intersection_of(circle<T> const c1, circle<T> const c2) {
//    auto const l2 = dot(c1.p - c2.p);
//    auto const r2 = c1.r * c1.r + c2.r * c2.r;
//
//    return {l2 < r2, {c1, c2}};
//}
//
//
//
//template <typename T>
//std::pair<point2d<T>, point2d<T>>
//reorder_x(point2d<T> p, point2d<T> q) {
//    return (p.x <= q.x) ? std::make_pair(p, q) : std::make_pair(q, p);
//}
//
//template <typename T>
//std::pair<point2d<T>, point2d<T>>
//reorder_y(point2d<T> p, point2d<T> q) {
//    return (p.y <= q.y) ? std::make_pair(p, q) : std::make_pair(q, p);
//}
//
//template <typename T>
//std::pair<bool, point2d<T>>
//intersect(point2d<T> p, point2d<T> q) {
//    return (p == q) ? std::make_pair(true, p) : std::make_pair(false, p);
//}
//
//template <typename T, typename U, typename V>
//point2d<T> translate(point2d<T> const p, U const dx, V const dy) {
//    return {p.x + dx, p.y + dy};
//}
//
//
//
//template <typename T>
//bool operator==(axis_aligned_rect<T> p, axis_aligned_rect<T> q) {
//    static_assert(std::is_integral<T>::value, "not implemented"); //TODO
//
//    return p.left()   == q.left()
//        && p.right()  == q.right()
//        && p.top()    == q.top()
//        && p.bottom() == q.bottom();
//}
//
//template <typename T>
//std::pair<bool, axis_aligned_rect<T>>
//intersect(axis_aligned_rect<T> const ra, axis_aligned_rect<T> const rb) {
//    return ra.intersect(rb);
//}
//
//template <typename T, typename U, typename V>
//axis_aligned_rect<T> translate(axis_aligned_rect<T> const r, U const dx, V const dy) {
//    return axis_aligned_rect<T>(translate(r.top_left(), dx, dy), r.width(), r.height());
//}
//
////==============================================================================
//template <typename T>
//std::pair<bool, point2d<T>>
//intersect(axis_aligned_rect<T> const r, point2d<T> const p) {
//    return {
//        p.x >= r.left() && p.x < r.right() && p.y >= r.top() && p.y < r.bottom()
//      , p
//    };
//}
//
//template <typename T>
//std::pair<bool, point2d<T>>
//intersect(point2d<T> const p, axis_aligned_rect<T> const r) {
//    return intersect(r, p);
//}

} //namespace bklib
