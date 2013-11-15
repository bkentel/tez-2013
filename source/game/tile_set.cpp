#include "pch.hpp"
#include "tile_set.hpp"

using namespace tez;

tile::tile(Json::Value const& json) {
    using namespace bklib::json;

    static utf8string const FIELD_NAME        = {"name"};
    static utf8string const FIELD_DESCRIPTION = {"description"};
    static utf8string const FIELD_LOCATION    = {"location"};
    static utf8string const FIELD_WEIGHT      = {"weight"};

    required_object(json);

    cref var_name   = json[FIELD_NAME];
    cref var_desc   = json[FIELD_DESCRIPTION];
    cref var_loc    = json[FIELD_LOCATION];
    cref var_weight = json[FIELD_WEIGHT];

    name        = tez::language_map(var_name);
    description = tez::language_map(var_desc);

    required_array(var_loc, 2, 2);
    location.x = required_integer(var_loc[0]);
    location.y = required_integer(var_loc[1]);

    weight = required_integer(var_weight);
}

//////////////////////////
tile_group::tile_group(Json::Value const& json) {
    using namespace bklib::json;

    static utf8string const FIELD_ID        = {"id"};
    static utf8string const FIELD_VARIATION = {"variation"};

    required_object(json);

    for (cref var : required_array(json[FIELD_VARIATION])) {
        tiles.emplace_back(tile{var});
    }

    id = required_string(json[FIELD_ID]);
}


//////////////////////////////

tile_set::tile_set() {
    using namespace bklib::json;

    static utf8string const FIELD_FILE_ID   = {"file_id"};
    static utf8string const FIELD_TILE_SIZE = {"tile_size"};
    static utf8string const FIELD_FILE_NAME = {"file_name"};
    static utf8string const FIELD_COLOR_KEY = {"color_key"};
    static utf8string const FIELD_TILES     = {"tiles"};

    Json::Value  json_root;
    Json::Reader json_reader;
    
    auto json_in = std::ifstream{R"(./data/tiles.def)"};
    if (!json_in) {
        //failed to open the file
        BK_DEBUG_BREAK();
    }

    if (!json_reader.parse(json_in, json_root)) {
        //failed to parse the file
        BK_DEBUG_BREAK();
        std::cout << json_reader.getFormattedErrorMessages();
    }

    required_object(json_root);

    if (required_string(json_root[FIELD_FILE_ID]) != FIELD_TILES) {
        //bad file id
        BK_DEBUG_BREAK();
    }

    size = required_integer<size_t>(json_root[FIELD_TILE_SIZE]);
    file_name = required_string(json_root[FIELD_FILE_NAME]);

    cref val_color_key = required_array(json_root[FIELD_COLOR_KEY], 3, 3);
    required_integer<uint8_t>(val_color_key[0]);
    required_integer<uint8_t>(val_color_key[1]);
    required_integer<uint8_t>(val_color_key[2]);

    auto hasher = tez::string_hasher{};
    for (cref val_group : required_array(json_root[FIELD_TILES])) {
        auto group = tile_group{val_group};
        auto const hash = hasher(group.id);

        tiles.emplace(hash, std::move(group));
    }
}
