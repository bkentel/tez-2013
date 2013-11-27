#include "pch.hpp"
#include "hotkeys.hpp"
#include "json.hpp"

using bklib::utf8string;
using bklib::hash;
using bklib::key_combo;
using tez::game_command;

namespace json = bklib::json;

utf8string const tez::hotkeys::DEFAULT_FILE_NAME = {"./data/bindings.def"};

namespace {
namespace local_state {
//------------------------------------------------------------------------------
template <typename K, typename V>
using flat_map = boost::container::flat_map<K, V>;

bool initialized = false;

flat_map<key_combo, game_command> combo_to_command;
flat_map<hash,      game_command> hash_to_command;
//------------------------------------------------------------------------------
#define BK_ADD_COMMAND_STRING(CMD)\
[] {\
    auto const hash = bklib::utf8string_hash(#CMD);\
    auto const result = hash_to_command.insert(std::make_pair(hash, CMD));\
    BK_ASSERT(result.second && "collision");\
}()

void init_command_strings() {
    BOOST_LOG_TRIVIAL(trace) << "initializing command strings.";

    BK_ASSERT(combo_to_command.empty() && hash_to_command.empty());

    using COMMAND = tez::game_command;

    auto const size = static_cast<size_t>(COMMAND::SIZE);
    hash_to_command.reserve(size);

    BK_ADD_COMMAND_STRING(COMMAND::USE);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_NORTH_WEST);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_NORTH);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_NORTH_EAST);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_WEST);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_HERE);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_EAST);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_SOUTH_WEST);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_SOUTH);
    BK_ADD_COMMAND_STRING(COMMAND::DIR_SOUTH_EAST);
}

#undef BK_ADD_COMMAND_STRING
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//// ROOT       -> {"bindings" : BINDINGS}
////// BINDINGS   -> [BINDING*]
////// BINDING    -> [COMMAND, COMBO_LIST]
////// COMMAND    -> string
////// COMBO_LIST -> [COMBO*]
////// COMBO      -> [KEY+]
////// KEY        -> string
////struct bindings_parser {
////    using cref       = bklib::json::cref;
////    using utf8string = bklib::utf8string;
////
////    //--------------------------------------------------------------------------
////    //! ROOT
////    //--------------------------------------------------------------------------
////    static void rule_root(cref json_root) {
////        static utf8string const KEY_NAME {"bindings"};
////
////        bklib::json::require_object(json_root);
////        cref json_bindings = bklib::json::require_key(json_root, KEY_NAME);
////        rule_bindings(json_bindings);
////    }
////    //--------------------------------------------------------------------------
////    //! BINDINGS
////    //--------------------------------------------------------------------------
////    static void rule_bindings(cref json_bindings) {
////        bklib::json::for_each_element_skip_on_fail(json_bindings, [&](cref json_binding) {
////            rule_binding(json_binding);
////        });
////    }
////    //--------------------------------------------------------------------------
////    //! BINDING
////    //--------------------------------------------------------------------------
////    static void rule_binding(cref json_binding) {
////        bklib::json::require_array(json_binding);
////
////        auto const size = json_binding.size();
////        if (size < 2) {
////            BK_DEBUG_BREAK(); //TODO
////        } else if (size > 2) {
////            BK_DEBUG_BREAK(); //TODO
////        }
////
////        cref json_command    = bklib::json::require_key(json_binding, 0);
////        cref json_combo_list = bklib::json::require_key(json_binding, 1);
////
////        auto command = tez::json::factory::make_game_command(json_command);
////        rule_combo_list(json_combo_list);
////    }
////    //--------------------------------------------------------------------------
////    //! COMMAND
////    //--------------------------------------------------------------------------
////    static tez::game_command rule_command(cref json_command) {
////        return tez::json::factory::make_game_command(json_command);
////    }
////    //--------------------------------------------------------------------------
////    //! COMBO_LIST
////    //--------------------------------------------------------------------------
////    static void rule_combo_list(cref json_combo_list) {
////        bklib::json::for_each_element_skip_on_fail(json_combo_list, [&](cref json_combo) {
////            auto combo = bklib::json::factory::make_key_combo(json_combo);
////        });
////    }
////};
//
////------------------------------------------------------------------------------
//void init_hotkeys(std::istream& in) {
//    static utf8string const ROOT_KEY = {"bindings"};
//
//    auto const command_count = static_cast<size_t>(game_command::SIZE);
//    combo_to_command.clear();
//    combo_to_command.reserve(command_count);
//
//    //auto state = bklib::json::input_stack(in);
//
//    //rule_root(state);
//}
////------------------------------------------------------------------------------
//void init() {
//    BOOST_LOG_TRIVIAL(trace) << "initializing hotkeys.";
//
//    init_command_strings();
//    initialized = true;
//    hotkeys::reload();
//}
////------------------------------------------------------------------------------
//} //namespace local_state
//} //namespace
//
////------------------------------------------------------------------------------
//void hotkeys::reload(utf8string const& file) {
//    std::ifstream in {file};
//    reload(in);
//}
//
//void hotkeys::reload(std::istream& in) {
//    if (!in) {
//        BK_DEBUG_BREAK(); //TODO
//    }
//
//    local_state::init_hotkeys(in);
//}
//
//game_command hotkeys::command_from_string(utf8string const& name) {
//    return command_from_hash(utf8string_hash(name));
//}
//
//game_command hotkeys::command_from_hash(hash name) {
//    if (!local_state::initialized) { local_state::init(); }
//    return local_state::command_from_hash(name);
//}
//
//game_command hotkeys::command_from_combo(key_combo const& combo) {
//    if (!local_state::initialized) { local_state::init(); }
//    return find_or(local_state::combo_to_command, combo, game_command::NONE);
//}
////------------------------------------------------------------------------------
//
//tez::game_command
//tez::json::factory::make_game_command(bklib::json::cref json_command) {
//    auto const name    = bklib::json::require_string(json_command);
//    auto const command = tez::hotkeys::command_from_string(name);
//
//    if (command == tez::game_command::NONE) {
//        BOOST_LOG_TRIVIAL(warning)
//         << "unknown command name."
//         << "name = " << name
//        ;
//    }
//
//    return command;
//}

} //namespace local_state
} //namespace

tez::game_command tez::hotkeys::command_from_hash(bklib::hash name) {
    return find_or(local_state::hash_to_command, name, game_command::NONE);
}

game_command tez::hotkeys::command_from_string(utf8string const& name) {
    return command_from_hash(bklib::utf8string_hash(name));
}

////////////////////
// parser
////////////////////

namespace {
void add_rule_exception_info(json::error::base& e, utf8string const& rule) {
    auto const ptr = boost::get_error_info<json::error::info_location>(e);

    if (ptr) {
        ptr->insert(0, rule);
    } else {
        e << json::error::info_location(rule);
    }
}
} //namespace
//------------------------------------------------------------------------------
tez::bindings_parser::bindings_parser(utf8string const& file_name)
  : bindings_parser {std::ifstream {file_name}}
{    
}

tez::bindings_parser::bindings_parser(std::istream& in)
  : bindings_parser {std::move(in)}
{
}

tez::bindings_parser::bindings_parser(std::istream&& in) {
    if (!in) {
        BK_DEBUG_BREAK(); //TODO
    }

    Json::Reader reader;
    Json::Value  root;

    auto const result = reader.parse(in, root);
    if (!result) {
        BK_DEBUG_BREAK(); //TODO
    }

    rule_root(root);
}

//------------------------------------------------------------------------------
void tez::bindings_parser::rule_root(cref json_root) {
    try {
        static utf8string const root_key {"bindings"};

        json::require_object(json_root);
        cref json_binding_list = json::require_key(json_root, root_key);

        rule_binding_list(json_binding_list);
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "ROOT");
        throw;
    }
}
//------------------------------------------------------------------------------
void tez::bindings_parser::rule_binding_list(cref json_binding_list) {
    try {
        json::require_array(json_binding_list);
    
        json::for_each_element_skip_on_fail(json_binding_list, [&](cref json_binding) {
            rule_binding(json_binding);
        });
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "BINDING_LIST");
        throw;
    }
}
//------------------------------------------------------------------------------
void tez::bindings_parser::rule_binding(cref json_binding) {
    try {
        json::require_array(json_binding);

        auto const size = json_binding.size();
        if (size < 2) {
        } else if (size > 2) {
        }

        cref json_command    = json::require_key(json_binding, 0);
        cref json_combo_list = json::require_key(json_binding, 1);

        cur_command_ = rule_command(json_command);
        if (cur_command_ == game_command::NONE) {
            return; //TODO
        }

        rule_combo_list(json_combo_list);
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "BINDING");
        throw;
    }
}
//------------------------------------------------------------------------------
tez::game_command tez::bindings_parser::rule_command(cref json_command) {
    try {
        auto const command_string = json::require_string(json_command);
        auto const command = hotkeys::command_from_string(command_string);

        return command;
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "COMMAND");
        throw;
    }
}
//------------------------------------------------------------------------------
void tez::bindings_parser::rule_combo_list(cref json_combo_list) {
    try {
        json::require_array(json_combo_list);

        json::for_each_element_skip_on_fail(json_combo_list, [&](cref json_combo) {
            cur_combo_.clear();
            cur_combo_ = rule_combo(json_combo);
            if (cur_combo_.empty()) {
                BK_DEBUG_BREAK(); //TOOO
            }

            auto const result = bindings_.emplace(
                std::make_pair(std::move(cur_combo_), cur_command_)
            );

            if (!result.second) {
                BK_DEBUG_BREAK(); //TOOO
            }
        });
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "COMBO_LIST");
        throw;
    }
}
//------------------------------------------------------------------------------
bklib::key_combo tez::bindings_parser::rule_combo(cref json_combo) {
    try {
        json::require_array(json_combo);

        key_combo result;

        json::for_each_element_skip_on_fail(json_combo, [&](cref json_key) {
            auto const key = rule_key(json_key);
            if (key == bklib::keys::NONE) {
                BK_DEBUG_BREAK(); //TOOO
            }

            auto const added = result.add(key);
            if (!added) {
                BK_DEBUG_BREAK(); //TOOO
            }
        });

        return result;
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "COMBO");
        throw;
    }
}
//------------------------------------------------------------------------------
bklib::keys tez::bindings_parser::rule_key(cref json_key) {
    try {
        auto const key_string = json::require_string(json_key);
        auto const key = bklib::keyboard::key_code(key_string);

        return key;
    } catch (json::error::base& e) {
        add_rule_exception_info(e, "KEY");
        throw;
    }
}
//------------------------------------------------------------------------------
