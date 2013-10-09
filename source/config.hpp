#pragma once

#include <boost/predef.h>

#if defined(WIN32)
#   define BK_PLATFORM_WINDOWS
#endif

#if defined(_MSC_VER)
#   define BK_COMPILER_MSVC
#endif

#ifdef BK_COMPILER_MSVC
#   define BK_NOEXCEPT throw()
#   define BK_NOEXCEPT_OP(x)
#else
#   define BK_NOEXCEPT noexcept
#   define BK_NOEXCEPT_OP(x) noexcept(x)
#endif

#define BK_CAT2_IMPL(A1, A2) A1 ## A2
#define BK_CAT2(A1, A2) BK_CAT2_IMPL(A1, A2)

#if defined(BK_COMPILER_MSVC)
#   define BK_UNIQUE_ID(NAME) BK_CAT2(NAME, __COUNTER__)
#else
#   define BK_UNIQUE_ID(NAME) BK_CAT2(NAME, __LINE__)
#endif
