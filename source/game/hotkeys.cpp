#include "pch.hpp"
#include "hotkeys.hpp"

using namespace bklib;
using namespace tez;

utf8string const tez::hotkeys::DEFAULT_FILE_NAME = {"./data/bindings.def"};

namespace {
namespace local_state {
//------------------------------------------------------------------------------
template <typename K, typename V>
using flat_map = boost::container::flat_map<K, V>;

singleton_flag initialized;

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
game_command command_from_hash(hash name) {
    return find_or(local_state::hash_to_command, name, game_command::NONE);
}
//------------------------------------------------------------------------------
key_combo make_combo(json::input_stack& state) {
    BOOST_LOG_TRIVIAL(trace) << "building combo";

    key_combo result;

    for (size_t i = 0; i < state.size(); ++i) {
        auto const name = state.require_string(i);
        auto const key  = keyboard::key_code(name);

        if (key == keys::NONE) {
            BOOST_LOG_TRIVIAL(warning) << "unknown key: " << name;
        } else if (!result.add(key)) {
            BOOST_LOG_TRIVIAL(warning) << "duplicate key: " << name << "; ignored";
        }
    }

    return result;
}
//------------------------------------------------------------------------------
void add_bindings(utf8string const& command_name, json::input_stack& state) {
    BOOST_LOG_TRIVIAL(trace) << "adding bindings for command: " << command_name;

    auto command = command_from_hash(utf8string_hash(command_name));
    if (command == tez::game_command::NONE) {
        BOOST_LOG_TRIVIAL(warning) << "unknown command: " << command_name << "; ignored";
        return;
    }

    for (size_t i = 0; i < state.size(); ++i) {
        auto combo = make_combo(state.step_into(i));
        BK_SCOPE_EXIT({state.step_out();});

        if (combo.size() == 0) {
            BOOST_LOG_TRIVIAL(warning) << "no valid keys for command: " << command_name << "; ignored";
            continue;
        }

        auto result = combo_to_command.emplace(
            std::make_pair(std::move(combo), command)
        );

        if (!result.second) {
            BOOST_LOG_TRIVIAL(warning) << "duplicate combo for command: " << command_name << "; ignored";
        }
    }
}
//------------------------------------------------------------------------------
void init_hotkeys(std::istream& in) {
    static utf8string const ROOT_KEY = {"bindings"};

    auto const command_count = static_cast<size_t>(game_command::SIZE);
    combo_to_command.clear();
    combo_to_command.reserve(command_count);

    auto state = json::input_stack(in);
    if (state.size() != 1) {
       BOOST_LOG_TRIVIAL(warning) << "hotkeys: too many elements at root"; 
    }

    state.step_into(ROOT_KEY);

    for (size_t i = 0; i < state.size(); ++i) {
        state.step_into(i);
        BK_SCOPE_EXIT({state.step_out();});

        if (state.size() != 2) {
            BOOST_LOG_TRIVIAL(warning) << "hotkeys: too many elements."; 
        }

        auto command_string = state.require_string(0);

        state.step_into(1);
        BK_SCOPE_EXIT({state.step_out();});
        add_bindings(command_string, state);
    }
}
//------------------------------------------------------------------------------
void init() {
    BOOST_LOG_TRIVIAL(trace) << "initializing hotkeys.";

    init_command_strings();
    initialized.initialized = true;
    hotkeys::reload();
}
//------------------------------------------------------------------------------
} //namespace local_state
} //namespace

//------------------------------------------------------------------------------
void hotkeys::reload(utf8string const& file) {
    std::ifstream in {file};
    reload(in);
}

void hotkeys::reload(std::istream& in) {
    if (!in) {
        BK_DEBUG_BREAK(); //TODO
    }

    local_state::init_hotkeys(in);
}

game_command hotkeys::command_from_string(utf8string const& name) {
    return command_from_hash(utf8string_hash(name));
}

game_command hotkeys::command_from_hash(hash name) {
    if (!local_state::initialized) { local_state::init(); }
    return local_state::command_from_hash(name);
}

game_command hotkeys::command_from_combo(key_combo const& combo) {
    if (!local_state::initialized) { local_state::init(); }
    return find_or(local_state::combo_to_command, combo, game_command::NONE);
}
//------------------------------------------------------------------------------
