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

struct singleton_flag {
    bool initialized = false;
    explicit operator bool() const { return initialized; }
};

template <typename R, typename C, typename K>
R const& find_or(C const& container, K const& key, R const& fallback) {
    auto const it = container.find(key);

    return it != std::cend(container)
      ? it->second
      : fallback;
}

//==============================================================================
//TODO
//WARNING
//not thread safe
//==============================================================================
struct hotkeys {
    using key_combo = bklib::key_combo;
    using utf8string = bklib::utf8string;

    static utf8string const DEFAULT_FILE_NAME;

    static void reload(utf8string const& file = DEFAULT_FILE_NAME);
    static void reload(std::istream& in);

    static game_command command_from_combo(key_combo const& combo);
    static game_command command_from_string(utf8string const& name);
    static game_command command_from_hash(bklib::hash name);
};

} //namespace tez
