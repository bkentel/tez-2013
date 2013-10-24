#include "pch.hpp"

#include "room.hpp"

using room = tez::room;

room::room(
    index_t w
  , index_t h
  , tez::tile_data value
)
  : grid2d(w, h, value)
{
}

room::room(room&& other)
  : grid2d(std::move(other))
{
}

room& room::operator=(room&& rhs)
{
    rhs.swap(*this);
    return *this;
}

void room::swap(room& other) {
    using std::swap;
    swap(*this, other);
}

//==============================================================================

using room_simple = tez::generator::room_simple;

room_simple::room_simple(range w, range h)
  : width_{w.first, w.second}
  , height_{h.first, h.second}
{
}

room room_simple::generate(tez::random& rand) {
    auto const w = width_(rand);
    auto const h = height_(rand);

    auto const value = tez::tile_data {
        tez::tile_type::floor
      , 0
      , 0
    };

    auto result = room(w, h, value);

    std::replace_if(
        std::begin(result)
      , std::end(result),
        [&result](room::iterator::value_type const& it) {
            return (it.i.x == 0)
                || (it.i.y == 0)
                || (it.i.x == result.width()  - 1)
                || (it.i.y == result.height() - 1);
        }
      , tile_data {tez::tile_type::wall, 0, 0}
    );

    return result;
}
