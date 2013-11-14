#pragma once

#include <string>
#include <boost/container/flat_map.hpp>
#include "types.hpp"

namespace tez {

using bklib::utf8string;
using bklib::string_hasher;

using language_id = uint8_t;

//==============================================================================
//!
//==============================================================================
struct language_info {
    using hash = size_t;

    static utf8string const FILE_NAME;

    //! tuple<id, string_id, name>
    using info = std::tuple<
        language_id, utf8string, utf8string
    >;

    static info const& get_info(hash lang);
    static info const& get_info(utf8string const& lang);

    static bool is_defined(hash lang);
    static bool is_defined(utf8string const& lang);

    static language_id fallback();
    static language_id default();
    static utf8string  substitute();
};
//==============================================================================
//!
//==============================================================================
class language_map {
public:
    using hash = language_info::hash;

    explicit language_map(size_t size = 0);
    explicit language_map(Json::Value const& json);

    utf8string const& operator[](language_id id) const;

    void insert(language_id id, utf8string value);
private:
    using map = boost::container::flat_map<language_id, utf8string>;

    map values_;
};

} //namespace tez
