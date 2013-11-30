#pragma once

#include "types.hpp"

#define BK_DELETE_ALL(name)\
name(name const&) = delete;\
name(name&&) = delete;\
name& operator=(name const&) = delete;\
name& operator=(name&&) = delete

namespace bklib {

inline size_t utf8string_hash(char const* str) BK_NOEXCEPT {
    size_t result {5381};
    while (*str) {
        result = result * 33 ^ *str++;
    }
    return result;
}

inline size_t utf8string_hash(utf8string const& str) BK_NOEXCEPT {
    return utf8string_hash(str.c_str());
}

template <typename T, size_t N = sizeof(T)> struct print_size_of;
template <typename T> struct print_type_of;

template <typename Enum>
static inline auto enum_value(Enum const e) BK_NOEXCEPT {
    static_assert(std::is_enum<Enum>::value, "must be an enum type.");
    return static_cast<std::underlying_type_t<Enum>>(e);
}

//==============================================================================
// Enum -> Bit flags helper.
//==============================================================================
template <typename EnumType>
class bit_flags {
public:
    static_assert(std::is_enum<EnumType>::value, "must be an enum type.");

    using storage_t = std::underlying_type_t<EnumType>;
    BK_CONSTEXPR static size_t const bits = sizeof(storage_t)*8;

    static storage_t get_value(EnumType const value) BK_NOEXCEPT {
        auto result = static_cast<storage_t>(value);
        if (result == 0) return 0;
        else result--;

        BK_ASSERT(result < bits);
        return 1 << result;
    }

    inline bit_flags() BK_NOEXCEPT
        : value_{0}
    {
    }

    inline bit_flags(EnumType const flag) BK_NOEXCEPT
        : value_{get_value(flag)}
    {
    }

    inline bool operator&(EnumType const flag) const BK_NOEXCEPT {
        return (value_ & get_value(flag)) != 0;
    }

    inline bit_flags operator|(EnumType const flag) const BK_NOEXCEPT {
        return {value_ | get_value(flag)};
    }

    inline bit_flags operator|=(EnumType const flag) BK_NOEXCEPT {
        return (*this = *this | flag);
    }

    inline void reset() BK_NOEXCEPT { value_ = 0; }
    inline void reset(EnumType const flag) BK_NOEXCEPT { value_ = get_value(flag); }
private:
    bit_flags(storage_t const value) BK_NOEXCEPT : value_{value} {}
    storage_t value_;
};

//==============================================================================
template <typename R, typename C, typename K>
R const& find_or(C const& container, K const& key, R const& fallback) {
    auto const it = container.find(key);

    return it != std::cend(container)
      ? it->second
      : fallback;
}


} //namespace bklib
