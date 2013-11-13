#include "pch.hpp"
#include "direct2d.hpp"

#pragma comment(lib, "D2d1.lib")

using namespace bklib::win;

#define BK_THROW_IF_FAILED_COM(function, hresult)\
    while (FAILED(hresult)) {\
        BOOST_THROW_EXCEPTION(::bklib::win::com_error {}\
            << boost::errinfo_api_function(#function)\
            << boost::errinfo_errno(hresult)\
        );\
    } []() -> void {}()

namespace {
    com_ptr<ID2D1Factory> create_factory() {
        ID2D1Factory* factory = nullptr;

        HRESULT const hr = ::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory
        );

        BK_THROW_IF_FAILED_COM(D2D1CreateFactory, hr);

        return com_ptr<ID2D1Factory>(factory);
    }

    com_ptr<ID2D1HwndRenderTarget>
    create_renderer(ID2D1Factory& factory, HWND window) {
        // Obtain the size of the drawing area.
        RECT window_rect {0};
        ::GetClientRect(window, &window_rect);

        // Create a Direct2D render target			
        ID2D1HwndRenderTarget* target = nullptr;			
        HRESULT const hr = factory.CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                window,
                D2D1::SizeU(
                    window_rect.right  - window_rect.left,
                    window_rect.bottom - window_rect.top
                )
            ),
            &target
        );
        
        BK_THROW_IF_FAILED_COM(CreateHwndRenderTarget, hr);

        return com_ptr<ID2D1HwndRenderTarget>(target);
    }

    com_ptr<ID2D1SolidColorBrush>
    create_brush(ID2D1HwndRenderTarget& target) {
        ID2D1SolidColorBrush* brush = nullptr;
        HRESULT const hr = target.CreateSolidColorBrush(
            D2D1::ColorF(1.0f, 1.0f, 1.0f),
            &brush
        );

        BK_THROW_IF_FAILED_COM(CreateSolidColorBrush, hr);

        return com_ptr<ID2D1SolidColorBrush>(brush);
    }
}

d2d_renderer::d2d_renderer(HWND window)
  : x_off_{0.0f}, y_off_{0.0f}
  , x_scale_{1.0f}, y_scale_{1.0f}
  , factory_(create_factory())
  , target_(create_renderer(*factory_, window))
  , brush_(create_brush(*target_))
{
}
