#include "pch.hpp"
#include "window.hpp"

//using pseudo_random_t = std::mt19937;
//using true_random_t = std::random_device;
//
//enum class axis : unsigned {
//    x, y, z, w
//};
//
//enum class direction : unsigned {
//    here, nw, n, ne, w, e, sw, s, se
//};
//
//namespace impl {
//    static int const BK_X_ARRAY[] = {
//        0,
//        -1, 0, 1,
//        -1, 1,
//        -1, 0, 1,
//    };
//
//    static int const BK_Y_ARRAY[] = {
//        0,
//        -1, -1, -1,
//        0, 0, 0,
//        1, 1, 1,
//    };
//
//    unsigned direction_value(direction dir) BK_NOEXCEPT {
//        return static_cast<std::underlying_type<direction>::type>(dir);
//    }
//
//    int delta_x(direction dir) BK_NOEXCEPT {
//        return BK_X_ARRAY[direction_value(dir)];
//    }
//
//    int delta_y(direction dir) BK_NOEXCEPT {
//        return BK_Y_ARRAY[direction_value(dir)];
//    }
//}
//
//template <typename T>
//class point2d {
//public:
//    point2d(T x, T y) : x(x), y(y) {}
//    point2d(point2d const&) = default;
//    point2d& operator=(point2d const&) = default;
//    T x, y;
//};
//
//template <typename T>
//class rect {
//public:
//    rect(point2d<T> p, T width, T height)
//        : origin_ {p}
//        , width {width}
//        , height {height}
//    {
//    }
//
//    rect(T top, T left, T bottom, T right)
//        : origin_ {top, left}
//        , width_ {right - left}
//        , height_ {bottom - top}
//    {
//    }
//
//    T left()   const BK_NOEXCEPT {return origin_.x;}
//    T right()  const BK_NOEXCEPT {return left() + width();}
//    T top()    const BK_NOEXCEPT {return origin_.y;}
//    T bottom() const BK_NOEXCEPT {return top() + height();}
//    T width()  const BK_NOEXCEPT {return width_;}
//    T height() const BK_NOEXCEPT {return height_; }
//
//    T area() const BK_NOEXCEPT {return width() * height();}
//private:
//    point2d<T> origin_;
//    T width_;
//    T height_;
//};
//
//template <typename T, typename U>
//rect<U> separate_rects_x(rect<T> r_static, rect<U> r) {
//
//}
//
//template <typename T, typename U>
//bool intersects(point2d<T> p, point2d<U> q) {
//    return p.x == q.x && p.y == q.y;
//}
//
//template <typename T, typename U>
//bool intersects(point2d<T> p, rect<U> r) {
//    return p.x >= r.left() && p.x < r.right() &&
//           p.y >= r.top()  && p.y < r.bottom();
//}
//
//template <typename T, typename U>
//bool intersects(rect<T> r, rect<U> s) {
//    return !(r.left()   >= s.right() ||
//             r.right()  <  s.left() ||
//             r.top()    >= s.bottom() ||
//             r.bottom() <  s.top());
//}
//
//template <typename T, typename U>
//bool intersects(rect<U> r, point2d<T> p) {
//    return intersects(p, r);
//}
//
//template <typename T, typename U>
//auto intersection_of(point2d<T> p, point2d<U> q)
//    -> std::pair<bool, point2d<decltype(p.x + q.x)>>
//{
//    return {intersects(p, q), {p.x, p.y}};
//}
//
//template <typename T, typename U>
//auto intersection_of(point2d<T> p, rect<U> r)
//    -> std::pair<bool, point2d<T>>
//{
//    return {intersects(p, r), p};
//}
//
//template <typename T, typename U>
//auto intersection_of(rect<U> r, point2d<T> p)
//    ->std::pair<bool, point2d<T>>
//{
//    return intersection_of(p, r);
//}
//
//template <typename T, typename U>
//auto intersection_of(rect<T> r, rect<U> s)
//    -> std::pair<bool, rect<T>>
//{   
//    return {
//        intersects(r, s),
//        {
//            r.top() < s.top() ? s.top() : r.top(),
//            r.left() < s.left() ? s.left() : r.left(),
//            r.bottom() < s.bottom() ? s.bottom() : r.bottom(),
//            r.right() < s.right() ? s.right() : r.right()
//        }
//    };
//}
//
//template <typename T>
//point2d<T> get_adjacent(point2d<T> p, direction dir) BK_NOEXCEPT {
//    return {
//        p.x + impl::delta_x(dir),
//        p.y + impl::delta_y(dir)
//    };
//}
//
//template <typename T>
//class grid {
//public:
//    grid(grid&& other)
//        : width_(other.width_)
//        , height_(other.height_)
//        , data_(std::move(other.data_))
//    {
//    }
//
//    grid& operator=(grid&& rhs) {
//        using std::swap;
//
//        swap(width_,  rhs.width_);
//        swap(height_, rhs.height_);
//        swap(data_,   rhs.data_);
//
//        return *this;
//    }
//
//    grid(unsigned width, unsigned height, T const& value
//        )
//        : width_(width)
//        , height_(height)
//        , data_(width*height, value)
//    {
//    }
//
//    grid(unsigned width, unsigned height,
//        typename std::enable_if<std::is_default_constructible<T>::value>::type* = nullptr
//    )
//        : width_(width)
//        , height_(height)
//        , data_(width*height)
//    {
//    }
//
//    T const& at(int x, int y) const {
//        return data_[get_index_(x, y)];
//    }
//
//    T& at(int x, int y) {
//        return data_[get_index_(x, y)];
//    }
//
//    unsigned width() const BK_NOEXCEPT {
//        return width_;
//    }
//
//    unsigned height() const BK_NOEXCEPT {
//        return height_;
//    }
//private:
//    size_t get_index_(int x, int y) const BK_NOEXCEPT {
//        BK_ASSERT(x >= 0 && y >= 0);
//        BK_ASSERT(x < width_ && y < height_);
//
//        return y * width_ + x;
//    }
//
//    int       width_;
//    int       height_;
//    std::vector<T> data_;
//
//};
//
//template <typename T, typename F>
//void for_each_xy(grid<T>& g, F function) {
//    for (unsigned y = 0; y < g.height(); ++y) {
//        for (unsigned x = 0; x < g.width(); ++x) {
//            function(g.at(x, y), x, y);
//        }
//    }
//}
//
//enum class tile_type : char {
//    empty,
//    floor,
//    wall,
//    door,
//};
//
//grid<tile_type> generate_rect_room(pseudo_random_t& gen, unsigned min = 2, unsigned max = 10) {
//    auto dist = std::uniform_int_distribution<unsigned>(min, max);
//    
//    unsigned width  = dist(gen);
//    unsigned height = dist(gen);
//
//    grid<tile_type> result = { width, height, tile_type::floor };
//
//    for_each_xy(result, [&](tile_type& t, unsigned x, unsigned y) {
//        if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
//            t = tile_type::wall;
//        }
//    });
//
//    return result;
//}


void main()
try {
    bklib::platform_window win {L"Tez"};

    auto on_mouse_move = [&](int x, int y) {
        std::cout << x << ", " << y << std::endl;
    };

    win.listen(
        bklib::mouse::on_move_to {on_mouse_move}
    );

    while (true) {
        win.do_events();
    }
} catch (...) {
    
}

