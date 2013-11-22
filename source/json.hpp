#pragma once

#include <type_traits>
#include <jsoncpp/json.h>

#include "types.hpp"
#include "exception.hpp"

namespace bklib { namespace json {

using value_t = Json::Value;
using cref    = Json::Value const&;
using field_t = utf8string const&;

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
inline T required_integer(cref value, field_t field) {
    auto integer = value[field];

    if (!integer.isIntegral()) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    }

    return get_integer<T>(integer);
}

template <typename T = int>
inline T required_integer(cref value, size_t index) {
    auto integer = value[index];

    if (!integer.isIntegral()) {
        BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    }

    return get_integer<T>(integer);
}

template <typename T = int>
inline T optional_integer(cref value, T fallback) {
    return value.isIntegral() ? get_integer<T>(value) : fallback;
}

inline utf8string optional_string(cref value, utf8string fallback) {
    return value.isString() ? value.asString() : fallback;
}

inline utf8string required_string(cref value, field_t field) {
    auto string = value[field];

    if (!string.isString()) {
        BOOST_THROW_EXCEPTION(json::bad_type());
    }

    return string.asString();
}

inline utf8string required_string(cref value, size_t index) {
    auto string = value[index];

    if (!string.isString()) {
        BOOST_THROW_EXCEPTION(json::bad_type());
    }

    return string.asString();
}

inline cref required_object(cref value) {
    if (!value.isObject()) {
        BOOST_THROW_EXCEPTION(json::bad_type());
    }

    return value;
}

inline cref required_array(
    cref array
  , size_t min_size = 0
  , size_t max_size = 0
) {
    if (!array.isArray()) {
        BOOST_THROW_EXCEPTION(json::bad_type());
    } else if (min_size != 0 && array.size() < min_size) {
        BOOST_THROW_EXCEPTION(json::bad_size());
    } else if (max_size != 0 && array.size() > max_size) {
        BOOST_THROW_EXCEPTION(json::bad_size());
    }

    return array;
}

inline value_t required_array(
    cref value
  , field_t field
  , size_t min_size = 0
  , size_t max_size = 0
) {
    return required_array(value[field], min_size, max_size);
}


////--------
template <typename T>
struct index_t {
    std::reference_wrapper<T const> index;

    cref operator()(cref value) const {
        return value[index];
    }
};

template <>
struct index_t<void> {
    cref operator()(cref value) const BK_NOEXCEPT {
        return value;
    }
};

template <typename T>
inline auto at_index(T const& index) {
    index_t<T> result = {std::cref(index)};
    return result;
}

inline index_t<void> at_index() BK_NOEXCEPT {
    return {};
}

template <Json::ValueType T>
struct type_t {
    static size_t const order = 1;
    static Json::ValueType const type = T;

    cref operator()(cref value) const {
        if (type != value.type()) {
            BOOST_THROW_EXCEPTION(bad_type());
        }

        return value;
    }
};

using require_string = type_t<Json::stringValue>;
using require_array = type_t<Json::arrayValue>;
using require_object = type_t<Json::objectValue>;

struct always_true {
    template <typename T>
    decltype(auto) operator()(T&& value) const {
        return std::forward<T>(value);
    }
};

template <typename T = void, typename F = always_true>
inline utf8string required_string_t(cref src, index_t<T> where = index_t<T>{}, F check = F{}) {
    cref result = check(
        require_string{}(
            where(src)
        )
    );

    return result.asString();
}

struct size_is {
    size_t size;

    cref operator()(cref value) const {
        if (value.size() != size) {
            BOOST_THROW_EXCEPTION(bad_size());
        }

        return value;
    }
};

template <typename T = void, typename F = always_true>
inline cref required_array_t(cref src, index_t<T> where = index_t<T>{}, F check = F{}) {
    cref result = check(
        require_array{}(
            where(src)
        )
    );

    return result;
}

inline cref required_object_t(cref src) {
    return require_object{}(src);
}

} //namespace json
} //namespace bklib
