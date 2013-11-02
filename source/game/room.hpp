#pragma once

#include "config.hpp"
#include "assert.hpp"
#include "tile_data.hpp"
#include "grid2d.hpp"

namespace tez {

using random = std::mt19937;

//==============================================================================
class room : public grid2d<tile_data> {
public:
    using rect  = bklib::axis_aligned_rect<int>;
    using point = bklib::point2d<int>;

    room(room const&) = delete;
    room& operator=(room const&) = delete;

    room(room&& other);
    room& operator=(room&& rhs);
    void swap(room& other);

    room(index_t w, index_t h,
         tile_data value = tile_data {tile_type::empty, 0, 0});
    
    rect get_rect() const {
        auto const w = static_cast<int>(width());
        auto const h = static_cast<int>(height());

        return {rect::tl_point{0, 0}, w, h};
    }
private:
};

namespace generator {
//==============================================================================
struct room_simple_fixed {
    room_simple_fixed(unsigned w, unsigned h)
      : width_{w}, height_{h} {}

    room generate(random& rand) const;

    unsigned width_;
    unsigned height_;
};
//==============================================================================
struct room_simple {
    using distribution = std::uniform_int_distribution<size_t>;
    using range = std::pair<size_t, size_t>;

    room_simple(range w, range h)
      : width_{w.first, w.second}, height_{h.first, h.second} {}

    room generate(random& rand);

    distribution width_;
    distribution height_;
};

namespace detail {

template <typename T>
struct intersection_iterator_traits {
    using base = typename std::vector<T>::iterator;
    using value = T;
    using traversal = boost::forward_traversal_tag;
    using reference = T&;
};

} //namespace detail

//==============================================================================
template <typename T>
class intersection_iterator : public boost::iterator_adaptor<
    intersection_iterator<T>                                    // Derived
  , typename detail::intersection_iterator_traits<T>::base      // Base
  , typename detail::intersection_iterator_traits<T>::value     // Value
  , typename detail::intersection_iterator_traits<T>::traversal // CategoryOrTraversal
  , typename detail::intersection_iterator_traits<T>::reference // Reference
> {
public:
    //intersection_iterator() BK_NOEXCEPT
    //  : iterator_adaptor_ {}
    //{
    //}

    intersection_iterator(base_type beg, base_type end, T comparison)
      : iterator_adaptor_ {beg}
      , comparison_{comparison}
      , end_{end}
    {
        base_reference() = find_next_(base_reference());
    }

    explicit intersection_iterator(base_type end)
      : iterator_adaptor_ {end}
      , comparison_{}
      , end_{end}
    {
    }
private:
    friend class boost::iterator_core_access;

    base_type find_next_(base_type where) const {
        return std::find_if(where, end_, [&](value_type const& x) {
            return bklib::intersects(x, comparison_);
        });
    }

    T& dereference() const {
        return *base_reference();
    }

    void advance(typename super_t::difference_type n) {
        while (base_reference() != end_ && n--) increment();
    }

    void increment() {
        base_reference() = find_next_(base_reference() + 1);
    }

    T         comparison_;
    base_type end_;
};


struct layout_random {
    using distribution = std::uniform_int_distribution<int>;
    using rect = bklib::axis_aligned_rect<int>;

    layout_random()
      : x_range_{-10, 10}
      , y_range_{-10, 10}
    {
    }

    void adjust_ranges(rect const r) {
        auto const min_x = std::min(r.left(), x_range_.min());
        auto const max_x = std::max(r.right(), x_range_.max());

        auto const min_y = std::min(r.top(), y_range_.min());
        auto const max_y = std::max(r.bottom(), y_range_.max());

        x_range_.param({min_x, max_x});
        y_range_.param({min_y, max_y});
    }

    struct looped_index {
        looped_index(random& rand, size_t const size)
          : cur{std::uniform_int_distribution<size_t>(0, size - 1)(rand)}
          , size{size - 1}
          , count{0}
        {
            BK_ASSERT(size > 0);
        }

        size_t operator++() {
            count++;
            return (cur = (cur + 1) % size);
        }

        operator size_t() const { return cur; }

        size_t cur;
        size_t size;
        size_t count;
    };

    void insert(random& rand, room&& new_room) {
        auto const insert_room = [&](rect r) {
            rects_.emplace_back(r);
            data_.emplace_back(std::move(new_room));
        };

        auto const ibegin = [&](rect const r) {
            return intersection_iterator<rect>(std::begin(rects_), std::end(rects_), r);
        };

        auto const iend = [&](rect const r) {
            return intersection_iterator<rect>(std::end(rects_));
        };

        auto const w = new_room.width();
        auto const h = new_room.height();
        auto const r = bklib::magnitude(bklib::vector2d<float>{{w / 2.0f, h / 2.0f}});

        auto const max_iter = rects_.size() * 2;

        if (rects_.empty()) {
            insert_room(rect(rect::tl_point{0, 0}, w, h));
            return;
        }

        auto index = looped_index {rand, rects_.size()};
        for (auto i = index; index.count < max_iter; ++i) {
            auto const cur_rect   = rects_[i];
            auto const cur_circle = bklib::bounding_circle(cur_rect);
            
            auto const v = bklib::random_direction(rand) * (r + cur_circle.r);
            auto const center = cur_circle.p + v;

            auto test_rect = rect(rect::center_point(bklib::round_toward<int>(center, v)), w, h);
            
            for (auto count = 0; count < 10; ++count) {
                static auto const zero = bklib::vector2d<float>{0.0f, 0.0f};

                auto const test_circle = bklib::bounding_circle(test_rect);

                auto const correction = std::accumulate(
                    ibegin(test_rect), iend(test_rect), zero
                  , [&](bklib::vector2d<float> v, rect ir) {
                        BK_ASSERT(bklib::intersects(ir, test_rect));

                        auto const ic = bklib::bounding_circle(ir);
                        auto const dir = bklib::direction(test_circle.p - ic.p);

                        std::cout << dir.x << " " << dir.y << std::endl;

                        auto const mag = -bklib::distance(test_circle, ic);
                        return v + dir * mag;
                    }
                );

                if (correction != zero) {
                    test_rect = test_rect + bklib::round_toward<int>(correction);
                    //std::cout << correction.x << " " << correction.y << std::endl;
                } else {
                    insert_room(test_rect);
                    return;
                }
            }
        }

        return;
    }

    distribution x_range_;
    distribution y_range_;

    std::vector<rect> rects_;
    std::vector<room> data_;
};
//==============================================================================
} //namespace genertor

} //namespace tez
