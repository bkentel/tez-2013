#pragma once

#include "keyboard.hpp"
#include "json.hpp"

namespace tez {

enum class game_command {
    NONE
  , USE
  , DIR_NORTH_WEST
  , DIR_NORTH
  , DIR_NORTH_EAST
  , DIR_WEST
  , DIR_HERE
  , DIR_EAST
  , DIR_SOUTH_WEST
  , DIR_SOUTH
  , DIR_SOUTH_EAST

  , SIZE //!< not a command; size of the enum only.
};

struct hotkeys {
    using key_combo = bklib::key_combo;

    static game_command translate(key_combo const& combo);
};

} //namespace tez
