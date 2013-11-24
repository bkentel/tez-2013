#pragma once

#include <type_traits>
#include <jsoncpp/json.h>

#include "types.hpp"
#include "exception.hpp"

namespace bklib { namespace json {

using value_t = Json::Value;
using cref    = Json::Value const&;
using field_t = utf8string const&;

using index_t = boost::variant<size_t, utf8string>;

namespace detail {
    utf8string const& get_type_string(Json::ValueType type);

    struct tag_info_expected_type;
    struct tag_info_actual_type;

    struct tag_info_expected_size;
    struct tag_info_actual_size;

    struct tag_info_index;
    struct tag_info_location;
} //namespace detail

namespace error {
    struct base : virtual exception_base {};
    struct bad_type  : virtual base {};
    struct bad_size  : virtual base {};
    struct bad_index : virtual base {};

    using info_expected_type = boost::error_info<detail::tag_info_expected_type, utf8string>;
    using info_actual_type   = boost::error_info<detail::tag_info_actual_type,   utf8string>;

    using info_expected_size = boost::error_info<detail::tag_info_expected_size, size_t>;
    using info_actual_size   = boost::error_info<detail::tag_info_actual_size,   size_t>;

    using info_index         = boost::error_info<detail::tag_info_index,         index_t>;
    using info_location      = boost::error_info<detail::tag_info_location,      utf8string>;

    std::ostream& operator<<(std::ostream& out, base const& e);

    inline bad_type make_type_info(
        Json::ValueType const expected
      , Json::ValueType const actual
    ) {
        bad_type e;

        e << info_expected_type{detail::get_type_string(expected)}
          << info_actual_type{detail::get_type_string(actual)};

        return e;
    }
} //namespace error

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
        //BOOST_THROW_EXCEPTION(bklib::json::bad_type());
    }

    return get_integer<T>(integer);
}

template <typename T = int>
inline T required_integer(cref value, size_t index) {
    auto integer = value[index];

    if (!integer.isIntegral()) {
        //BOOST_THROW_EXCEPTION(bklib::json::bad_type());
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
        //BOOST_THROW_EXCEPTION(json::bad_type());
    }

    return string.asString();
}

inline utf8string required_string(cref value, size_t index) {
    auto string = value[index];

    if (!string.isString()) {
        //BOOST_THROW_EXCEPTION(json::bad_type());
    }

    return string.asString();
}

inline cref required_object(cref value) {
    if (!value.isObject()) {
        //BOOST_THROW_EXCEPTION(json::bad_type());
    }

    return value;
}

inline cref required_array(
    cref array
  , size_t min_size = 0
  , size_t max_size = 0
) {
    if (!array.isArray()) {
        //BOOST_THROW_EXCEPTION(json::bad_type());
    } else if (min_size != 0 && array.size() < min_size) {
        //BOOST_THROW_EXCEPTION(json::bad_size());
    } else if (max_size != 0 && array.size() > max_size) {
        //BOOST_THROW_EXCEPTION(json::bad_size());
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

class input_stack {
public:
    using pointer_t = Json::Value const*;
    using record_t  = std::pair<index_t, pointer_t>;

    explicit input_stack(std::istream& in);

    explicit input_stack(std::istream&& in)
      : input_stack(in)
    {
    }

    explicit input_stack(utf8string const& filename)
      : input_stack(std::ifstream{filename})
    {
    }

    input_stack& step_into(index_t const& index);

    input_stack& step_out();

    utf8string require_string(index_t const& index) const;

    size_t size() const {
        return current_->size();
    }

    friend std::ostream& operator<<(std::ostream& lhs, input_stack const& rhs);
private:
    pointer_t get_index_(index_t const& index) const;

    Json::Value           root_;
    pointer_t             current_;
    std::vector<record_t> stack_; 
};

std::ostream& operator<<(std::ostream& lhs, input_stack const& rhs);

} //namespace json
} //namespace bklib
