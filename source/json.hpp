#pragma once

#include <type_traits>
#include <jsoncpp/json.h>

#include "types.hpp"
#include "exception.hpp"

namespace bklib { namespace json {

using cref = Json::Value const&;

struct error : virtual exception_base {};
struct bad_type : virtual exception_base {};
struct bad_size : virtual exception_base {};

template <typename To, typename From>
inline To truncate_cast(From value,
    typename std::enable_if<
        (std::is_signed<To>::value   && std::is_signed<From>::value)
     || (std::is_unsigned<To>::value && std::is_unsigned<From>::value)
    >::type* = 0
) {
    auto const masked = value & static_cast<To>(~0);

    if (masked != value) {
        std::cout << "warning: truncating value";
        return static_cast<To>(~0);
    }

    return static_cast<To>(value);
}

template <typename T>
inline T get_integer(cref json_value,
    typename std::enable_if<
        std::is_unsigned<T>::value && !std::is_same<T, uint64_t>::value
    >::type* = 0
) {
    BK_ASSERT(json_value.isIntegral());
    return truncate_cast<T>(json_value.asUInt());
}

template <typename T>
inline T get_integer(cref json_value,
    typename std::enable_if<
        std::is_signed<T>::value && !std::is_same<T, int64_t>::value
    >::type* = 0
) {
    BK_ASSERT(json_value.isIntegral());
    return truncate_cast<T>(json_value.asInt());
}

template <typename T>
T get_integer(cref value,
    typename std::enable_if<std::is_same<T, uint64_t>::value>::type* = 0
) {
    BK_ASSERT(value.isIntegral());
    return value.asUInt64();
}

template <typename T>
T get_integer(cref value,
    typename std::enable_if<std::is_same<T, int64_t>::value>::type* = 0
) {
    BK_ASSERT(value.isIntegral());
    return value.asInt64();
}

template <typename T = int>
inline T required_integer(cref value) {
    if (!value.isIntegral()) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    }

    return get_integer<T>(value);
}

template <typename T = int>
inline T optional_integer(cref value, T fallback) {
    return value.isIntegral() ? get_integer<T>(value) : fallback;
}

inline utf8string optional_string(cref value, utf8string fallback) {
    return value.isString() ? value.asString() : fallback;
}

inline utf8string required_string(cref value) {
    if (!value.isString()) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    }

    return value.asString();
}

inline cref required_object(cref value) {
    if (!value.isObject()) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    }

    return value;
}

inline cref required_array(cref value, size_t min_size = 0, size_t max_size = 0) {
    if (!value.isArray()) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    } else if (min_size != 0 && value.size() < min_size) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_size());
    } else if (max_size != 0 && value.size() > max_size) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_size());
    }

    return value;
}

} //namespace json
} //namespace bklib
