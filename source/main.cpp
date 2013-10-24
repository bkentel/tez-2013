#include "pch.hpp"
#include "window.hpp"

#include <boost/iterator/iterator_adaptor.hpp>

#include "math.hpp"

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

#pragma region temp

//template <typename T, typename IndexType>
//struct grid_itererator_value {
//    grid_itererator_value operator=(grid_itererator_value const&) = delete;
//
//    grid_itererator_value(T& value, index2d<IndexType> i) BK_NOEXCEPT
//        : value {value}, i {i}
//    {
//    }
//
//    operator T&() BK_NOEXCEPT { return value; }
//    operator T const&() const BK_NOEXCEPT { return value; }
//
//    T& value;
//    index2d<IndexType> i;
//};
//
//template <typename T, typename I>
//struct grid_itererator_traits {
//    using base      = typename std::vector<T>::iterator;
//    using value     = grid_itererator_value<T, I>;
//    using traversal = boost::random_access_traversal_tag;
//    using reference = value;
//};
//
//template <typename T, typename I>
//struct grid_itererator_traits<T const, I> {
//    using base      = typename std::vector<T>::const_iterator;
//    using value     = grid_itererator_value<T const, I>;
//    using traversal = boost::random_access_traversal_tag;
//    using reference = value;
//};
//
//template <typename T, typename IndexType>
//class grid_itererator
//  : public boost::iterator_adaptor<
//        grid_itererator<T, IndexType>                            // Derived
//      , typename grid_itererator_traits<T, IndexType>::base      // Base
//      , typename grid_itererator_traits<T, IndexType>::value     // Value
//      , typename grid_itererator_traits<T, IndexType>::traversal // CategoryOrTraversal
//      , typename grid_itererator_traits<T, IndexType>::reference // Reference
//    >
//{
//public:
//    grid_itererator() BK_NOEXCEPT
//      : grid_itererator::iterator_adaptor_ {}
//      , width_ {0}, height_ {0}, pos_ {}
//    {
//    }
//
//    grid_itererator(base_type it, IndexType w, IndexType h)
//      : grid_itererator::iterator_adaptor_ {it}
//      , width_ {w}, height_ {h}, pos_ {0}
//    {
//    }
//
//    template <typename U>
//    grid_itererator(
//        grid_itererator<U, IndexType> const& other
//      , typename std::enable_if<std::is_convertible<U*,T*>::value>::type* = nullptr
//    )
//        : grid_itererator::iterator_adaptor_(other.base())
//    {
//    }
// private:
//    friend class boost::iterator_core_access;
//
//    typename iterator_adaptor::reference dereference() const {
//        return grid_itererator_value<T, IndexType>(
//            *base(), {
//                static_cast<IndexType>(pos_) % width_,
//                static_cast<IndexType>(pos_) / width_
//            }
//        );
//    }
//
//    void advance(typename iterator_adaptor::difference_type n) {
//        base_reference() = base() + n;
//        pos_ += n;
//    }
//
//    void decrement() {
//        advance(-1);
//    }
//
//    void increment() {
//        advance(1);
//    }
//
//    size_t    pos_;
//    IndexType width_;
//    IndexType height_;
//};
//
//template <typename T, typename IndexType>
//class grid_sub_itererator
//  : public boost::iterator_adaptor<
//        grid_sub_itererator<T, IndexType>                            // Derived
//      , typename grid_itererator_traits<T, IndexType>::base      // Base
//      , typename grid_itererator_traits<T, IndexType>::value     // Value
//      , typename grid_itererator_traits<T, IndexType>::traversal // CategoryOrTraversal
//      , typename grid_itererator_traits<T, IndexType>::reference // Reference
//    >
//{
//public:
//    grid_sub_itererator() BK_NOEXCEPT
//      : grid_sub_itererator::iterator_adaptor_ {}
//      , width_ {0}, height_ {0}, pos_ {}
//    {
//    }
//
//    grid_sub_itererator(base_type it, size_t offset, size_t stride, IndexType w, IndexType h)
//      : grid_sub_itererator::iterator_adaptor_ {it + offset}
//        , width_ {w}, height_ {h}, pos_ {0}, offset_ {offset}, stride_ {stride}
//    {
//    }
//
//    template <typename U>
//    grid_sub_itererator(
//        grid_sub_itererator<U, IndexType> const& other
//      , typename std::enable_if<std::is_convertible<U*,T*>::value>::type* = nullptr
//    )
//        : grid_sub_itererator::iterator_adaptor_(other.base())
//    {
//    }
// private:
//    friend class boost::iterator_core_access;
//
//    typename iterator_adaptor::reference dereference() const {
//        auto const x = (pos_ % width_) + (stride_ - width_);
//        auto const y = (pos_ + offset_) / stride_;
//
//        return grid_itererator_value<T, IndexType>(
//            *base(), {static_cast<IndexType>(x), static_cast<IndexType>(y) }
//        );
//    }
//
//    void advance(typename iterator_adaptor::difference_type n) {
//        auto dx = (pos_ + n) % width_;
//        auto dy = ((pos_ + n) / width_) - (pos_ / width_);
//
//        BK_ASSERT(pos_ + n >= 0);
//        BK_ASSERT(pos_ + n <= width_ + height_);
//
//        if (pos_ + n == width_ + height_) {
//            base_reference() = base() + n;
//        } else {
//            base_reference() = base() + n + dy*(stride_ - width_ + 1);
//        }
//
//        pos_ += n;
//    }
//
//    void decrement() {
//        advance(-1);
//    }
//
//    void increment() {
//        advance(+1);
//    }
//
//    size_t pos_;
//    size_t offset_;
//    size_t stride_;
//
//    IndexType width_;
//    IndexType height_;
//};
//
//
//template <typename T>
//index2d<T> north(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
//    return {i.x, i.y - n};
//}
//
//template <typename T>
//index2d<T> south(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
//    return {i.x, i.y + n};
//}
//
//template <typename T>
//index2d<T> east(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
//    return {i.x + n, i.y};
//}
//
//template <typename T>
//index2d<T> west(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
//    return {i.x - n, i.y};
//}
//
//template <typename T>
//T const& as_const(T& x) BK_NOEXCEPT {
//    return x;
//}
//
//
//class level_map {
//    using rect = axis_aligned_rect<int>;
//
//    rect find_space_for(rect ra) const {
//        ra.move_to(rect::point {{0, 0}});
//
//        for (auto const& rb : room_rects_) {
//            auto i = intersect(ra, rb);
//
//            if (i.first) {
//
//            } else {
//                return ra;
//            }
//        }
//    }
//
//    grid2d<tile>      grid_;
//    std::vector<rect> room_rects_;
//};
#pragma endregion temp

using point  = bklib::point2d<int>;
using rect   = bklib::axis_aligned_rect<int>;
using random = std::mt19937;

template <typename T>
struct quad_tree {
    struct detail {
        using index_t = std::uint16_t;
        static auto const size_i = sizeof(index_t);

        using index_vector = std::vector<index_t>;
        static auto const size_v = sizeof(index_vector);

        static auto const size_total = size_v + size_v % sizeof(std::max_align_t) + sizeof(std::max_align_t);

        static auto const array_count = size_total / size_i - 1;
        using index_array = std::array<index_t, array_count>;

        using storage_t = std::aligned_storage_t<size_total>;

        static index_t const UNUSED = 0xFFFF;
    };

    struct child {
        typename detail::storage_t storage;

        child() {
            set_is_leaf();

            auto const beg = reinterpret_cast<detail::index_t*>(&storage);
            auto const end = reinterpret_cast<detail::index_t*>(&storage) + detail::array_count;

            std::fill(beg, end, detail::UNUSED);
        }

        void set_is_leaf(bool value = true) {
            reinterpret_cast<detail::index_t*>(&storage)[detail::array_count] = (value ? 1 : 0);
        }

        bool is_leaf() const {
            return reinterpret_cast<detail::index_t*>(&storage)[detail::array_count] == 0;
        }

        typename detail::index_array& as_array() {
            return *reinterpret_cast<detail::index_array*>(&storage);
        }

        typename detail::index_array& as_vector() {
            BK_ASSERT(!is_leaf());
            return *reinterpret_cast<detail::index_vector*>(&storage);
        }
    };

    using rect = bklib::axis_aligned_rect<int>;

    explicit quad_tree(rect bounds)
        : bounds_{bounds}
    {
        std::cout << detail::size_i      << std::endl;
        std::cout << detail::size_v      << std::endl;
        std::cout << detail::size_total  << std::endl;
        std::cout << detail::array_count << std::endl;
    }

    void insert(rect r, T data) {

    }

    child root_;

    std::vector<T>     data_;
    std::vector<child> children_;

    rect bounds_;
};


struct simple_room_gen {
    using distr = std::uniform_int_distribution<unsigned>;

    simple_room_gen(unsigned min_w, unsigned max_w, unsigned min_h, unsigned max_h)
      : dist_w_(min_w, max_w)
      , dist_h_(min_h, max_h)
    {
        BK_ASSERT(min_w <= max_w);
        BK_ASSERT(min_h <= max_h);
    }

    distr dist_w_;
    distr dist_h_;
};


struct random_placement_generator {
    using random = std::mt19937;
    using distr  = std::uniform_int_distribution<int>;
    using rect   = bklib::axis_aligned_rect<int>;

    random_placement_generator(
        int      range_x, int      range_y,
        unsigned min_w,   unsigned max_w,
        unsigned min_h,   unsigned max_h
    )
      : distribution_x_(-range_x, range_x)
      , distribution_y_(-range_y, range_y)
      , distribution_w_(min_w, max_w)
      , distribution_h_(min_h, max_h)
      , rects_{}
    {
        BK_ASSERT(range_x > 0);
        BK_ASSERT(range_y > 0);
    }

    std::pair<bool, rect> find_intersection(rect r, unsigned from = 0) const {
        std::pair<bool, rect> result;

        std::find_if(
            std::cbegin(rects_) + from,
            std::cend(rects_),
            [&](rect s) -> bool { return (result = bklib::intersect(r, s)).first; }
        );

        return result;
    }

    bool generate(random& rand) {
        auto const x = distribution_x_(rand);
        auto const y = distribution_y_(rand);
        auto const w = distribution_w_(rand);
        auto const h = distribution_h_(rand);

        auto const l = x;
        auto const t = y;
        auto const r = x + w;
        auto const b = y + h;

        rect const room_rect {
            l
          , t
          , r < distribution_x_.max() ? r : distribution_x_.max()
          , b < distribution_y_.max() ? b : distribution_y_.max()
        };

        auto ri = find_intersection(room_rect);

        if (ri.first) {
            BK_DEBUG_BREAK();
        }

        rects_.emplace_back(room_rect);

        return true;
    }

    distr distribution_x_;
    distr distribution_y_;
    distr distribution_w_;
    distr distribution_h_;

    std::vector<rect> rects_;
};

void main()
try {
    random rand(100);

    auto gen = random_placement_generator(100, 100, 2, 10, 2, 10);

    for (int i = 0; i < 25; ++i) {
        gen.generate(rand);
    }

    point p1 {10, 10};
    point p2 {10, 10};

    auto r = (p1 == p2);

    auto const pi = bklib::intersect(p1, p2);

    rect r1 {0, 0, 10, 10};
    rect r2 {10, 10, 15, 15};

    auto ri = bklib::intersect(r1, r2);


    bklib::platform_window win {L"Tez"};

    auto on_mouse_move = [&](int x, int y) {
        std::cout << x << ", " << y << std::endl;
    };

    win.listen(
        bklib::mouse::on_move_to {on_mouse_move}
    );

    while (win.is_running()) {
        win.do_events();
    }

    auto result = win.result_value().get();

    std::cout << "done!";
} catch (...) {
    std::cout << "oops!";
}

