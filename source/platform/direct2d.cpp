#include "pch.hpp"
#include "direct2d.hpp"

#pragma comment(lib, "D2d1.lib")

namespace impl = ::bklib::impl;
using d2d_renderer = impl::d2d_renderer;

namespace {
    impl::com_ptr<ID2D1Factory> create_factory() {
        ID2D1Factory* factory = nullptr;

        HRESULT const hr = ::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory
        );

        if (FAILED(hr)) {
            throw "TODO";
        }

        return impl::com_ptr<ID2D1Factory>(factory);
    }

    impl::com_ptr<ID2D1HwndRenderTarget>
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
        if (FAILED(hr)) {
            throw "TODO";
        }

        return impl::com_ptr<ID2D1HwndRenderTarget>(target);
    }

    impl::com_ptr<ID2D1SolidColorBrush>
    create_brush(ID2D1HwndRenderTarget& target) {
        ID2D1SolidColorBrush* brush = nullptr;
        HRESULT const hr = target.CreateSolidColorBrush(
            D2D1::ColorF(1.0f, 0.0f, 0.0f),
            &brush
        );

        if (FAILED(hr)) {
            throw "TODO";
        }

        return impl::com_ptr<ID2D1SolidColorBrush>(brush);
    }
}

d2d_renderer::d2d_renderer(HWND window)
    : factory_(create_factory())
    , target_(create_renderer(*factory_, window))
    , brush_(create_brush(*target_))
{
}
