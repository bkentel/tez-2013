#pragma once

#include "keyboard.hpp"
#include "json_forward.hpp"
#include "types.hpp"

namespace tez {

using bklib::utf8string;

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



template <typename R, typename C, typename K>
R const& find_or(C const& container, K const& key, R const& fallback) {
    auto const it = container.find(key);

    return it != std::cend(container)
      ? it->second
      : fallback;
}


//==============================================================================
// ROOT         -> {"bindings" : BINDING_LIST}
// BINDING_LIST -> [BINDING*]
// BINDING      -> [COMMAND, COMBO_LIST]
// COMMAND      -> string
// COMBO_LIST   -> [COMBO*]
// COMBO        -> [KEY+]
// KEY          -> string
//==============================================================================

class bindings_parser {
public:
    using map_t = boost::container::flat_map<bklib::key_combo, tez::game_command>;
    using cref  = bklib::json::cref;

    bindings_parser(utf8string const& file_name);
    bindings_parser(std::istream&& in);
    bindings_parser(std::istream& in);

    void             rule_root(         cref json_root);
    void             rule_binding_list( cref json_binding_list);
    void             rule_binding(      cref json_binding);
    game_command     rule_command(      cref json_command);
    void             rule_combo_list(   cref json_combo_list);
    bklib::key_combo rule_combo(        cref json_combo);
    bklib::keys      rule_key(          cref json_key);

private:
    game_command     cur_command_;
    bklib::key_combo cur_combo_;
    map_t            bindings_;
};

//==============================================================================
//TODO
//WARNING
//not thread safe
struct hotkeys {
    using key_combo  = bklib::key_combo;
    using utf8string = bklib::utf8string;

    static utf8string const DEFAULT_FILE_NAME;

    static void reload(utf8string const& file = DEFAULT_FILE_NAME);
    static void reload(std::istream& in);

    static game_command command_from_combo(key_combo const& combo);
    static game_command command_from_string(utf8string const& name);
    static game_command command_from_hash(bklib::hash name);
};

} //namespace tez

