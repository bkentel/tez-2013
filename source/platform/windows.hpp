#pragma once

//==============================================================================
//! @file
//! Include files for windows and windows libraries.
//==============================================================================

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>

#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "D2d1.lib")

#include <dwrite.h>
#pragma comment(lib, "Dwrite.lib")

#include <wincodec.h>
#pragma comment(lib, "Windowscodecs.lib")

#include <imm.h>
#pragma comment(lib, "Imm32.lib")

#include "config.hpp"
#include "exception.hpp"

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
