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
    room(room const&) = delete;
    room& operator=(room const&) = delete;

    room(room&& other);
    room& operator=(room&& rhs);
    void swap(room& other);

    room(index_t w, index_t h,
         tile_data value = tile_data {tile_type::empty, 0, 0});
private:
};

namespace generator {

struct room_simple {
    using distribution = std::uniform_int_distribution<size_t>;
    using range = std::pair<size_t, size_t>;

    room_simple(range w, range h);

    room generate(random& rand);

    distribution width_;
    distribution height_;
};

} //namespace genertor

} //namespace tez
