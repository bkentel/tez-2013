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
    }
 private:
    friend class boost::iterator_core_access;

    T& dereference() const {
        return *base_reference();
    }

    void advance(typename super_t::difference_type n) {
        while (base_reference() != end_ && n--) increment();
    }

    void increment() {
        base_reference() = std::find_if(
            base_reference()
          , end_
          , [&](value_type const& x) {
                return bklib::intersects(x, comparison_);
            }
        );
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

    size_t fill_intersections(rect const r) const {
        intersections_.clear();
    
        size_t count = 0;

        //for (auto const r0 : rects_) {
        //    auto const i = bklib::intersect(r, r0);
        //    if (i.first) {
        //        intersections_.emplace_back(i.second);
        //        ++count;
        //    }
        //}

        return count;
    }

    std::pair<size_t, rect> get_area_and_max() const {
        BK_ASSERT(!intersections_.empty());

        rect   max_rect;
        size_t max_area = 0;
        size_t area_sum = 0;

        //for (auto const r : intersections_) {
        //    auto const area = r.area();
        //    area_sum += area;

        //    if (area > max_area) {
        //        max_rect = r;
        //        max_area = area;
        //    }
        //}

        return {area_sum, max_rect};
    }

    rect find_location(rect const r) const {
        return {};

        //auto n = fill_intersections(r);
        //if (n == 0) return r;

        //bklib::point2d<float> vec = {0.0f, 0.0f};
        //
        //auto const p0 = r.center<float>();

        //for (auto const ir : intersections_) {
        //    auto const p1 = ir.center<float>();
        //    auto const v  = p0 - p1;
        //    auto const u  = bklib::normalize(v);
        //    vec += u;
        //}

        //vec = bklib::normalize(vec);

        //auto const cur   = get_area_and_max();
        //auto const& ir   = cur.second;
        //auto const& area = cur.first;

        //auto const p0 = r.center();
        //auto const p1 = ir.center();
        //auto const v = p0 - p1;

        //return {
        //    area, (ir.width() > ir.height())
        //      ? bklib::translate(r, bklib::sign_of(v.x) * ir.width(), 0)
        //      : bklib::translate(r, 0, bklib::sign_of(v.y) * ir.height())
        //};
    }

    rect locate_relative_to(rect const origin, rect const r, random& rand) {
        BK_ASSERT(r.left() == 0);
        BK_ASSERT(r.top()  == 0);

        auto const c1 = bklib::bounding_circle(origin);
        auto const c2 = bklib::bounding_circle(r);
        auto const v  = (c1.r + c2.r) * bklib::random_direction(rand);

        rect::center_point const p = bklib::round_toward<int>(c1.p + v, v);

        return {p, r.width(), r.height()};
    }

    void insert(random& rand, room&& new_room) {
        auto const new_rect   = new_room.get_rect();
        auto const new_circle = bklib::bounding_circle(new_rect);

        BK_ASSERT(new_rect.left() == 0);
        BK_ASSERT(new_rect.top() == 0);

        if (rects_.empty()) {
            rects_.emplace_back(new_rect);
            data_.emplace_back(std::move(new_room));
            return;
        }

        auto const size  = rects_.size();
        auto const first =
            std::uniform_int_distribution<size_t>(0, size)(rand);

        for (unsigned n = 0; n < size; ++n) {
            auto const i = (first + n) % size;

            auto const from_rect = rects_[i];
            auto const from_circle = bklib::bounding_circle(from_rect);


            auto test_rect   = locate_relative_to(rects_[i], new_rect, rand);
            auto test_circle = bklib::bounding_circle<float>(test_rect);

            auto const beg = std::begin(rects_);
            auto const end = std::end(rects_);

            bool can_insert = false;
            while (!can_insert) {
                auto it_beg = intersection_iterator<rect>(beg, end, test_rect);
                auto it_end = intersection_iterator<rect>(end, end, test_rect);

                auto where = it_beg + 1;

                if (where == it_end) {
                    can_insert = true;
                    break;
                }
 
                auto const ir = *where;
                auto const ic = bklib::bounding_circle<float>(ir);

                auto const dir  = bklib::direction<float>(test_circle.p - ic.p);
                auto const dist = -bklib::distance<float>(test_circle, ic);
                auto const v    = bklib::round_toward<int>(dist * dir);

                BK_ASSERT(dist >= 0);

                test_rect   = bklib::translate(test_rect, v);
                test_circle = bklib::bounding_circle<float>(test_rect);

                if (bklib::intersects(ir, test_rect)) {
                    auto result = bklib::intersection_of(ir, test_rect);
                    BK_DEBUG_BREAK();
                }
            }

            if (can_insert) {
                rects_.emplace_back(test_rect);
                data_.emplace_back(std::move(new_room));
                return;
            }
        }

        BK_DEBUG_BREAK();
    }

    distribution x_range_;
    distribution y_range_;

    std::vector<rect> rects_;
    std::vector<room> data_;

    std::vector<rect> mutable intersections_;
};
//==============================================================================
} //namespace genertor

} //namespace tez
