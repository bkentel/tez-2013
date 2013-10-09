#pragma once

#include <type_traits>
#include <memory>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include "config.hpp"
#include "com.hpp"

namespace bklib {
namespace win {

class d2d_renderer {
public:
    d2d_renderer(d2d_renderer const&) = delete;
    d2d_renderer& operator=(d2d_renderer const&) = delete;
    ~d2d_renderer() = default;

    d2d_renderer(HWND window);

    void resize(unsigned w, unsigned h) {
        target_->Resize(D2D1::SizeU(w, h));
    }

    void begin() {
        target_->BeginDraw();
    }

    void end() {
        HRESULT const hr = target_->EndDraw();
        if (FAILED(hr)) {
            BK_DEBUG_BREAK();
        }
    }

    void draw_filled_rect(float top, float left, float w, float h) {
        target_->FillRectangle(D2D1::RectF(left, top, left + w, top + h), brush_.get());
    }
private:
    com_ptr<ID2D1Factory>          factory_;
    com_ptr<ID2D1HwndRenderTarget> target_;

    com_ptr<ID2D1SolidColorBrush> brush_;
};

} //win
} //tez
