#pragma once

#include <cstdint>

namespace tez {

enum class tile_type : std::uint16_t {
    empty,
    floor,
    wall,
    door,
};

struct tile_data {
    tile_type     type;
    std::uint16_t sub_type;
    std::uint32_t data;
};

static_assert(sizeof(tile_data) == 8, "unexpected size");

} //namespace tez
