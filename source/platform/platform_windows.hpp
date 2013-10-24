#pragma once

#include "config.hpp"
#include "exception.hpp"

#if defined(BOOST_OS_WINDOWS)
#   include <Windows.h>
#   include <d2d1.h>
#   include <d2d1helper.h>
#   include <dwrite.h>
#   include <wincodec.h>
#   include <imm.h>
#   include <Unknwn.h>
#   include <ObjIdl.h>

namespace bklib { namespace win {

    struct windows_error : virtual platform_error {};
    struct com_error : virtual windows_error {};

}} //namespace bklib::win

#define BK_THROW_ON_COM_FAIL(function) \
for (auto const value = (function); FAILED(value);) { \
    BOOST_THROW_EXCEPTION(::bklib::win::com_error {} \
        << boost::errinfo_api_function(#function) \
        << boost::errinfo_errno(value) \
    ); \
} []{}

#define BK_THROW_WINAPI(api) \
for (;;) { \
    BOOST_THROW_EXCEPTION(::bklib::win::com_error {} \
        << boost::errinfo_api_function(#api) \
        << boost::errinfo_errno(::GetLastError()) \
    ); \
} []{}

#endif
