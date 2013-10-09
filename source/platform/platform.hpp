#pragma once

#include <exception>
#include <boost/exception/all.hpp>

#include "config.hpp"

#if defined(BK_PLATFORM_WINDOWS)
#   include <Windows.h>
#   include <d2d1.h>
#   include <d2d1helper.h>
#   include <dwrite.h>
#   include <wincodec.h>
#   include <imm.h>
#   include <Unknwn.h>

    struct platform_error : virtual std::exception, virtual boost::exception {};
#else
#endif
