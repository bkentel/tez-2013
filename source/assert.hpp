#pragma once

#include <exception>

#ifdef BK_COMPILER_MSVC
#   define BK_DEBUG_BREAK __debugbreak
#endif

#define BK_ASSERT(condition) \
do { \
    if (!(condition)) { \
        BK_DEBUG_BREAK(); \
        std::terminate(); \
    } \
} while (!(condition))
