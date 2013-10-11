#include "pch.hpp"
#include "window.hpp"

#include <boost/iterator/iterator_adaptor.hpp>

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

template <typename T>
struct index2d {
    T x, y;
};

template <typename T>
struct point2d {
    T x, y;
};

template <typename T>
void reorder(point2d<T>& pa, point2d<T>& pb) {
    using std::swap;    

    if (pb.x < pa.x) swap(pa.x, pb.x);
    if (pb.y < pa.y) swap(pa.y, pb.y);
}

template <typename T, typename IndexType>
struct grid_itererator_value {
    grid_itererator_value operator=(grid_itererator_value const&) = delete;    

    grid_itererator_value(T& value, index2d<IndexType> i) BK_NOEXCEPT
        : value {value}, i {i}
    {
    }

    operator T&() BK_NOEXCEPT { return value; }
    operator T const&() const BK_NOEXCEPT { return value; }

    T& value;
    index2d<IndexType> i;
};

template <typename T, typename I>
struct grid_itererator_traits {
    using base      = typename std::vector<T>::iterator;
    using value     = grid_itererator_value<T, I>;
    using traversal = boost::random_access_traversal_tag;    
    using reference = value;
};

template <typename T, typename I>
struct grid_itererator_traits<T const, I> {
    using base      = typename std::vector<T>::const_iterator;
    using value     = grid_itererator_value<T const, I>;
    using traversal = boost::random_access_traversal_tag;
    using reference = value;
};

template <typename T, typename IndexType>
class grid_itererator
  : public boost::iterator_adaptor<
        grid_itererator<T, IndexType>                            // Derived
      , typename grid_itererator_traits<T, IndexType>::base      // Base
      , typename grid_itererator_traits<T, IndexType>::value     // Value
      , typename grid_itererator_traits<T, IndexType>::traversal // CategoryOrTraversal
      , typename grid_itererator_traits<T, IndexType>::reference // Reference
    >
{
public:
    grid_itererator() BK_NOEXCEPT
      : grid_itererator::iterator_adaptor_ {}
      , width_ {0}, height_ {0}, pos_ {}
    {
    }

    grid_itererator(base_type it, IndexType w, IndexType h)
      : grid_itererator::iterator_adaptor_ {it}
      , width_ {w}, height_ {h}, pos_ {0}
    {
    }

    template <typename U>
    grid_itererator(
        grid_itererator<U, IndexType> const& other
      , typename std::enable_if<std::is_convertible<U*,T*>::value>::type* = nullptr
    )
        : grid_itererator::iterator_adaptor_(other.base())
    {
    }
 private:
    friend class boost::iterator_core_access;

    typename iterator_adaptor::reference dereference() const {
        return grid_itererator_value<T, IndexType>(
            *base(), {
                static_cast<IndexType>(pos_) % width_,
                static_cast<IndexType>(pos_) / width_
            }
        );
    }

    void advance(typename iterator_adaptor::difference_type n) {
        base_reference() = base() + n;
        pos_ += n;
    }

    void decrement() {
        advance(-1);
    }

    void increment() {
        advance(1);
    }
   
    size_t    pos_;
    IndexType width_;
    IndexType height_;
};

template <typename T, typename IndexType>
class grid_sub_itererator
  : public boost::iterator_adaptor<
        grid_sub_itererator<T, IndexType>                            // Derived
      , typename grid_itererator_traits<T, IndexType>::base      // Base
      , typename grid_itererator_traits<T, IndexType>::value     // Value
      , typename grid_itererator_traits<T, IndexType>::traversal // CategoryOrTraversal
      , typename grid_itererator_traits<T, IndexType>::reference // Reference
    >
{
public:
    grid_sub_itererator() BK_NOEXCEPT
      : grid_sub_itererator::iterator_adaptor_ {}
      , width_ {0}, height_ {0}, pos_ {}
    {
    }

    grid_sub_itererator(base_type it, size_t offset, size_t stride, IndexType w, IndexType h)
      : grid_sub_itererator::iterator_adaptor_ {it + offset}
        , width_ {w}, height_ {h}, pos_ {0}, offset_ {offset}, stride_ {stride}
    {
    }

    template <typename U>
    grid_sub_itererator(
        grid_sub_itererator<U, IndexType> const& other
      , typename std::enable_if<std::is_convertible<U*,T*>::value>::type* = nullptr
    )
        : grid_sub_itererator::iterator_adaptor_(other.base())
    {
    }
 private:
    friend class boost::iterator_core_access;

    typename iterator_adaptor::reference dereference() const {
        auto const x = (pos_ % width_) + (stride_ - width_);
        auto const y = (pos_ + offset_) / stride_;

        return grid_itererator_value<T, IndexType>(
            *base(), {static_cast<IndexType>(x), static_cast<IndexType>(y) }
        );
    }

    void advance(typename iterator_adaptor::difference_type n) {
        auto dx = (pos_ + n) % width_;
        auto dy = ((pos_ + n) / width_) - (pos_ / width_);

        BK_ASSERT(pos_ + n >= 0);
        BK_ASSERT(pos_ + n <= width_ + height_);

        if (pos_ + n == width_ + height_) {
            base_reference() = base() + n;
        } else {
            base_reference() = base() + dx + dy*(stride_ - width_ + 1);    
        }
        
        pos_ += n;
    }

    void decrement() {
        advance(-1);
    }

    void increment() {
        advance(+1);
    }
   
    size_t pos_;
    size_t offset_;
    size_t stride_; 

    IndexType width_;
    IndexType height_;
};

template <typename T, typename IndexType = int>
class grid2d {
public:
    using point = point2d<IndexType>;
    using index_t = point;

    using reference = T&;
    using const_reference = T const&;

    using iterator       = grid_itererator<T,       IndexType>;
    using const_iterator = grid_itererator<T const, IndexType>;

    using sub_iterator       = grid_sub_itererator<T,       IndexType>;
    using const_sub_iterator = grid_sub_itererator<T const, IndexType>;

    grid2d(IndexType w, IndexType h, T value = T {})
        : width_  { [&] { BK_ASSERT(w > 0); return w; }() }
        , height_ { [&] { BK_ASSERT(h > 0); return h; }() }
        , data_(static_cast<size_t>(w)*static_cast<size_t>(h), value)
    {
    }

    reference operator[](index_t i) {
        return data_[index2d_to_index_(i)];
    }

    const_reference operator[](index_t i) const {
        return data_[index2d_to_index_(i)];
    }

    bool is_valid(index_t i) const BK_NOEXCEPT {
        return (i.x >= 0)     && (i.y >= 0)
            && (i.x < width_) && (i.y < height_);
    }

    sub_iterator begin(point pa, point pb) {
        reorder(pa, pb);
        
        auto const w = pb.x - pa.x;
        auto const h = pb.y - pa.y;

        BK_ASSERT(w > 0);
        BK_ASSERT(h > 0);

        return sub_iterator(data_.begin(), index2d_to_index_(pa), width_, w, h);
    }
    
    sub_iterator end(point pa, point pb) {
        reorder(pa, pb);

        auto const w = pb.x - pa.x;
        auto const h = pb.y - pa.y;

        BK_ASSERT(w > 0);
        BK_ASSERT(h > 0);

        //the recct formed by pa and pb is exclusive of pb; make the iterator
        //point to one element past the last element.
        --pb.y;

        return sub_iterator(data_.begin(), index2d_to_index_(pb), 0, 0, 0);
    }
    
    iterator begin() { return iterator(data_.begin(), width_, height_); }
    iterator end() { return iterator(data_.end(), width_, height_); }

    const_iterator begin() const { return const_iterator(data_.begin(), width_, height_); }
    const_iterator end() const { return const_iterator(data_.end(), width_, height_); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
private:
    size_t index2d_to_index_(index_t i) const BK_NOEXCEPT {
        BK_ASSERT(is_valid(i));
        return i.y * width_ + i.x;
    }

    IndexType width_, height_;
    std::vector<T> data_;
};

template <typename T>
index2d<T> north(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
    return {i.x, i.y - n};
}
    
template <typename T>
index2d<T> south(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
    return {i.x, i.y + n};
}

template <typename T>
index2d<T> east(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
    return {i.x + n, i.y};
}

template <typename T>
index2d<T> west(index2d<T> i, typename std::make_signed<T>::type n = 1) BK_NOEXCEPT {
    return {i.x - n, i.y};
}

template <typename T>
T const& as_const(T& x) BK_NOEXCEPT {
    return x;
}



template <typename T>
class axis_aligned_rect {
public:
    using point = point2d<T>;

    axis_aligned_rect(T x0, T y0, T x1, T y1)
        : x0_ {x0}, y0_ {y0}
        , x1_ {x1}, y1_ {y1}
    {
        BK_ASSERT(x1 > x0);
        BK_ASSERT(y1 > y0);
    }

    axis_aligned_rect(std::initializer_list<T> list)
        : axis_aligned_rect(
            *(list.begin() + 0)
          , *(list.begin() + 1)
          , *(list.begin() + 2)
          , *(list.begin() + 3)
        )
    {
        BK_ASSERT(list.size() == 4);        
    }

    axis_aligned_rect(point p, T w, T h)
        : axis_aligned_rect(p.x, p.y, p.x + w, p.y + h)
    {
    }

    void move_to(point p) {
        auto const dx = p.x - x0_;
        auto const dy = p.y - y0_;

        x0_ += dx; x1_ += dx;
        y0_ += dy; y1_ += dy;
    }

    T left()   const BK_NOEXCEPT { return x0_; }
    T right()  const BK_NOEXCEPT { return x1_; }
    T top()    const BK_NOEXCEPT { return y0_; }
    T bottom() const BK_NOEXCEPT { return y1_; }

    T width()  const BK_NOEXCEPT { return right()  - left(); }
    T height() const BK_NOEXCEPT { return bottom() - top();  }
private:
    bool is_well_formed() const {
        return (left() < right()) && (top() < bottom());
    }

    T x0_, x1_;
    T y0_, y1_;
};

template <typename T>
std::pair<bool, axis_aligned_rect<T>>
intersect(axis_aligned_rect<T> const ra, axis_aligned_rect<T> const rb) {
    T const left   = std::max(ra.left(),   rb.left());
    T const right  = std::min(ra.right(),  rb.right());
    T const top    = std::max(ra.top(),    rb.top());
    T const bottom = std::min(ra.bottom(), rb.bottom());

    if (left < right && top < bottom) {
        return { true, {left, top, right, bottom} };
    } else {
        return { false, {0, 0, 1, 1} };
    }
}

struct tile {
    enum class tile_type : uint16_t {
        empty = 0,
    };

    tile_type type;
    uint64_t  data;
};

class level_map {
    using rect = axis_aligned_rect<int>;

    rect find_space_for(rect ra) const {
        ra.move_to(rect::point {{0, 0}});

        for (auto const& rb : room_rects_) {
            auto i = intersect(ra, rb);

            if (i.first) {
                
            } else {
                return ra;
            }
        }
    }

    grid2d<tile>      grid_;
    std::vector<rect> room_rects_;
};


void main()
try {
    grid2d<std::string> test_grid(10, 10, "Empty");

    for (auto cell : test_grid) {
        std::stringstream out;
        out << "Cell[" << cell.i.x << ", " << cell.i.y << "]";
        cell.value = out.str();
    }

    //for (auto cell : as_const(test_grid)) {
    //    std::cout << cell.value << std::endl;
    //}

    using point = grid2d<std::string>::point;
    auto beg = test_grid.begin(point {2, 2}, point {4, 4});
    auto end = test_grid.end(point {2, 2}, point {4, 4});

    for (auto it = beg; it != end; ++it) {
        auto const& cell = *it;
        std::cout << cell.value << std::endl;
    }

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

