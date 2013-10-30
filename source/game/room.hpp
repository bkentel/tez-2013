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

        point const p = {0, 0};

        return {p, w, h};
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
//==============================================================================
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

    void insert(random& rand, room&& new_room) {
        if (rects_.empty()) {
            rects_.emplace_back(new_room.get_rect());
            data_.emplace_back(std::move(new_room));
            return;
        }

        auto const size  = rects_.size();
        auto const begin =
            std::uniform_int_distribution<unsigned>(0, size)(rand);

        auto const rect_new   = new_room.get_rect();
        auto const bounds_new = bklib::bounding_circle(rect_new);

        for (unsigned n = 0; n < size; ++n) {
            auto const i      = (begin + n) % size;
            auto const bounds = bklib::bounding_circle(rects_[i]);
            auto const dir    = bklib::random_direction(rand);
            auto const v      = (bounds.r + bounds_new.r) * dir;

            auto const test_rect = rect_new + bklib::to_type<int>(v);

            bool const intersects = std::cend(rects_) != std::find_if(
                std::cbegin(rects_), std::cend(rects_), [&](rect const& r) {
                    return bklib::intersects(test_rect, r);
                }
            );

            if (!intersects) {
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
