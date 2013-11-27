#pragma once

#include <boost/variant.hpp>
#include "types.hpp"

namespace Json {
    class Value;
} //namespace Json

namespace bklib {
    namespace json {
        using cref  = Json::Value const&;
        using index = boost::variant<size_t, utf8string>;
    } //namespace json
} //namespace bklib
