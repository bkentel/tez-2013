#pragma once

#include "json.hpp"
#include "math.hpp"
#include "languages.hpp"

namespace tez {

//
struct tile {
    using location_t = bklib::point2d<int>;

    explicit tile(Json::Value const& json);

    tile(tile&& other)
      : name{std::move(other.name)}
      , description{std::move(other.description)}
      , location{other.location}
      , weight{other.weight}
    {
    }

    tile& operator=(tile&& rhs) {
        swap(rhs);
        return *this;
    }

    void swap(tile& other) {
        using std::swap;
        swap(name, other.name);
        swap(description, other.description);
        swap(location, other.location);
        swap(weight, other.weight);
    }

    language_map name;
    language_map description;
    location_t   location;
    unsigned     weight;
};

struct tile_group {
    tile_group(tile_group&& other)
      : id{other.id}, tiles{std::move(other.tiles)}
    {
    }

    tile_group& operator=(tile_group&& rhs) {
        swap(rhs);
        return *this;
    }

    void swap(tile_group& other) {
        using std::swap;
        swap(id, other.id);
        swap(tiles, other.tiles);
    }

    explicit tile_group(Json::Value const& json);

    utf8string        id;
    std::vector<tile> tiles;
};

struct tile_set {   
    tile_set();

    tile_set(tile_set&& other)
      : size{other.size}
      , file_name{std::move(other.file_name)}
      , tiles{std::move(other.tiles)}
    {
    }

    tile_set& operator=(tile_set&& rhs) {
        swap(rhs);
        return *this;
    }

    void swap(tile_set& other) {
        using std::swap;
        swap(size, other.size);
        swap(file_name, other.file_name);
        swap(tiles, other.tiles);
    }

    size_t     size;
    utf8string file_name;
    bklib::flat_map<size_t, tile_group> tiles;
};

} //namespace tez
