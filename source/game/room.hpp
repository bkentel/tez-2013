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

template <typename Container, typename Value, typename Test, typename Sum>
Value accumulate_if(Container& c, Value init, Test test, Sum sum) {
    auto it = std::cbegin(c);
    auto end = std::cend(c);

    Value value = init;

    while (end != (it = std::find_if(it, end, test))) {
        value = sum(value, *it++);
    }

    return value;
}

template <typename T>
struct min_max {
    min_max(T const initial = T{0})
      : min{initial}, max{initial}
    {
    }

    void operator()(T const n) {
        if (n < min) min = n;
        else if (n > max) max = n;
    }

    T min, max;
};

struct layout_random {
    using distribution = std::uniform_int_distribution<int>;
    using rect = bklib::axis_aligned_rect<int>;

    min_max<int> range_x_;
    min_max<int> range_y_;

    layout_random()
    {
    }

    struct looped_index {
        using dist = std::uniform_int_distribution<size_t>;

        looped_index(random& rand, size_t const size)
          : cur{dist(0, size ? size - 1 : 0)(rand)}
          , size{size ? size : 1}
        {
        }

        size_t operator++() { return ++cur % size; }
        operator size_t() const { return cur; }

        size_t cur, size;
    };

    void update_ranges(rect const& r) {
        range_x_(r.left());
        range_x_(r.right());
        range_y_(r.top());
        range_y_(r.bottom());
    }

    void insert(random& rand, room&& new_room) {
        using namespace bklib;
        static auto const zero = make_vector2d(0.0f, 0.0f);

        auto const w = new_room.width();
        auto const h = new_room.height();
        auto const r = magnitude(make_vector2d(w/2.0f, h/2.0f));

        auto test_rect = rect(rect::tl_point{0, 0}, w, h);
        bool inserted  = rects_.empty();

        auto get_correction = [&](rect const& test_rect) {
            auto const c = bounding_circle(test_rect);
            return accumulate_if(rects_, zero
                , [&](rect const& r) { return intersects(r, test_rect); }
                , [&](vector2d<float> const& v, rect const& ir) {
                    return v + separation_vector(c, bounding_circle(ir));
                }
            );
        };

        for (auto i = looped_index {rand, rects_.size()}; !inserted; ++i) {
            auto const cur_rect   = rects_[i];
            auto const cur_circle = bounding_circle(cur_rect);
            
            for (auto count = 0; !inserted && count < 10; ++count) {
                auto const v = random_direction(rand) * (r + cur_circle.r);
                auto const p = round_toward<int>(cur_circle.p + v, v);

                test_rect = rect(rect::center_point(p), w, h);

                auto correction = get_correction(test_rect);
                inserted = (zero == correction) || (zero == get_correction(
                    test_rect += round_toward<int>(correction)
                ));
            }
        }

        rects_.emplace_back(test_rect);
        data_.emplace_back(std::move(new_room));
    }

    std::vector<rect> rects_;
    std::vector<room> data_;
};
//==============================================================================
} //namespace genertor

} //namespace tez
