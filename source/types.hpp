#pragma once

#include <string>
#include <functional>

#include "config.hpp"

namespace bklib {

using int8_t  = ::std::int8_t;
using int16_t = ::std::int16_t;
using int32_t = ::std::int32_t;
using int64_t = ::std::int64_t;

using uint8_t  = ::std::uint8_t;
using uint16_t = ::std::uint16_t;
using uint32_t = ::std::uint32_t;
using uint64_t = ::std::uint64_t;

#if defined(BK_PLATFORM_WINDOWS)
    using platform_char   = wchar_t;
    using platform_string = std::basic_string<platform_char>;
#else
    using platform_char   = char;
    using platform_string = std::basic_string<char>;
#endif
    using invocable = std::function<void()>;
} //namespace bklib

//TODO
#define BK_UNUSED(x) (void)(x)