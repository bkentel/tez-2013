#pragma once

#include <boost/predef.h>

#if defined(BOOST_COMP_MSVC)
#   if BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(12,0,21005)
#       define BK_NOEXCEPT throw()
#       define BK_NOEXCEPT_OP(x)
#       define BK_CONSTEXPR
#   else if BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(13,0,0)
#       define BK_NOEXCEPT noexcept
#       define BK_NOEXCEPT_OP(x)
#       define BK_CONSTEXPR constexpr
#   endif
#endif

#define BK_CAT2_IMPL(A1, A2) A1 ## A2
#define BK_CAT2(A1, A2) BK_CAT2_IMPL(A1, A2)

#if defined(BOOST_COMP_MSVC)
#   define BK_UNIQUE_ID(NAME) BK_CAT2(NAME, __COUNTER__)
#else
#   define BK_UNIQUE_ID(NAME) BK_CAT2(NAME, __LINE__)
#endif

#define BK_UNUSED(x) (void)(x)
