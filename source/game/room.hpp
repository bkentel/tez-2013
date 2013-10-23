#pragma once

#include "config.hpp"
#include "assert.hpp"
#include "tile_data.hpp"
#include "grid2d.hpp"

namespace tez {

class room : public grid2d<tile_data> {
public:
    room(room const&) = delete;
    room& operator=(room const&) = delete;

    room(index_t w, index_t h, tile_data value = tile_data {tile_type::empty, 0, 0});
private:
};

} //namespace tez
