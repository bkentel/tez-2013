#include "pch.hpp"
#include "hotkeys.hpp"

using hk = tez::hotkeys;
using namespace bklib;

namespace {

std::once_flag once_flag;

boost::container::flat_map<bklib::key_combo, tez::game_command> hotkeys_map;
boost::container::flat_map<bklib::hash, tez::game_command> hash_command_map;

#define BK_ADD_COMMAND_STRING(CMD)\
[] {\
    auto const hash = bklib::utf8string_hash(#CMD);\
    auto const result = hash_command_map.insert(std::make_pair(hash, CMD));\
    BK_ASSERT(result.second && "collision");\
}()

void init_command_strings() {
    using COMMAND = tez::game_command;

    BK_ASSERT(hotkeys_map.size() == 0);
    BK_ASSERT(hash_command_map.size() == 0);

    hash_command_map.reserve(static_cast<size_t>(COMMAND::SIZE));
    hotkeys_map.reserve(static_cast<size_t>(COMMAND::SIZE));

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

tez::game_command get_command_from_string(utf8string const& name) {
    auto const hash = utf8string_hash(name);
    auto it = hash_command_map.find(hash);
    return it != std::cend(hash_command_map)
      ? it->second
      : tez::game_command::NONE; 
}

bklib::key_combo make_combo(bklib::json::cref combo) {
    json::required_array_t(combo);

    key_combo result;

    for (json::cref key : combo) {
        auto key_name = json::required_string_t(key);
        auto key = keyboard::key_code(key_name);

        if (key == keys::NONE) {
            BK_DEBUG_BREAK(); //TODO
        }

        result.add(key);
    }

    return result;
}

void add_bindings(bklib::utf8string command_name, bklib::json::cref combo_list) {
    json::required_array_t(combo_list);

    auto command = get_command_from_string(command_name);
    if (command == tez::game_command::NONE) {
        BK_DEBUG_BREAK(); //TODO
    }

    for (json::cref combo : combo_list) {
        auto result = hotkeys_map.emplace(std::make_pair(make_combo(combo), command));
        if (!result.second) {
            BK_DEBUG_BREAK(); //TODO
        }
    }
}

void init_hotkeys() {
    using namespace bklib;

    BK_ASSERT(hash_command_map.size() != 0);

    static utf8string const FILE_NAME = {"./data/bindings.def"};
    static utf8string const ROOT_KEY = {"bindings"};

    auto in = std::ifstream(FILE_NAME);
    if (!in) {
        BK_DEBUG_BREAK();
    }

    Json::Value  json_root;
    Json::Reader json_reader;
    
    if (!json_reader.parse(in, json_root)) {
        //failed to parse the file
        BK_DEBUG_BREAK();
        std::cout << json_reader.getFormattedErrorMessages();
    }

    json::required_object_t(json_root);
    json::cref bindings_list = json::required_array_t(json_root, json::at_index(ROOT_KEY));

    for (json::cref binding : bindings_list) {
        json::required_array_t(binding, json::at_index(), json::size_is{2});
        
        auto command_name = json::required_string_t(binding, json::at_index(0));
        auto combo_list = json::required_array_t(binding, json::at_index(1));

        add_bindings(command_name, combo_list);
    }
}

void init() {
    init_command_strings();
    init_hotkeys();
}

} //namespace

tez::game_command hk::translate(key_combo const& combo) {
    std::call_once(once_flag, init);

    auto const result = hotkeys_map.find(combo);
    return result != std::cend(hotkeys_map)
      ? result->second
      : game_command::NONE;
}
