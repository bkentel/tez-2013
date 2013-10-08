#pragma once

#include <string>
#include <functional>

#include "config.hpp"

namespace bklib {
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