#pragma once

#include <type_traits>
#include <memory>

#include <platform/platform_windows.hpp>

#include "config.hpp"
#include "com.hpp"

#include "math.hpp"

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

    void clear() {
        target_->Clear(D2D1::ColorF(1.0, 0.0, 0.0));
    }

    void translate(float dx, float dy) {
        x_off_ += dx;
        y_off_ += dy;
    }

    void scale(float s) {
        x_scale_ = s;
        y_scale_ = s;
    }

    void skew(float sx, float sy) {
        x_scale_ = sx;
        y_scale_ = sy;
    }

    template <typename T>
    void draw_filled_rect(bklib::axis_aligned_rect<T> const r) {
        auto mat = D2D1::Matrix3x2F(
            x_scale_, 0.0f
          , 0.0f,     y_scale_
          , x_off_,   y_off_
        );

        auto const rect = D2D1::RectF(r.left(), r.top(), r.right() - 1, r.bottom() - 1);

        target_->SetTransform(mat);
        target_->FillRectangle(rect, brush_.get());
        target_->DrawRectangle(rect, brush_.get());
        target_->SetTransform(D2D1::IdentityMatrix());
    }

    void draw_filled_rect(float top, float left, float w, float h) {    
        target_->FillRectangle(D2D1::RectF(left, top, left + w, top + h), brush_.get());
    }
private:
    float x_off_;
    float y_off_;
    float x_scale_;
    float y_scale_;

    com_ptr<ID2D1Factory>          factory_;
    com_ptr<ID2D1HwndRenderTarget> target_;
    com_ptr<ID2D1SolidColorBrush>  brush_;
};

} //win
} //tez
