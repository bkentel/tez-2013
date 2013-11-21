#include "pch.hpp"
#include "window.hpp"

#if defined(BOOST_OS_WINDOWS)
#   include "platform/window_windows.hpp"
#endif

using pw = bklib::platform_window;
using mouse = bklib::mouse;
using keyboard = bklib::keyboard;



////////////////////////////////////////////////////////////////////////////////
// bklib::platform_window
////////////////////////////////////////////////////////////////////////////////
pw::~platform_window() {
}

pw::platform_window(
    bklib::platform_string title
)
    : impl_ {new impl_t_ {}}
{
}

bool pw::is_running() const {
    return impl_->is_running();
}

std::future<int> pw::result_value() {
    return impl_->result_value();
}

void pw::do_events() {
    impl_->do_events();
}

pw::platform_handle pw::get_handle() const {
    return impl_->get_handle();
}

#define BK_DEFINE_EVENT(event)\
void pw::listen(event callback) {\
    impl_->listen(callback);\
}

BK_DEFINE_EVENT(on_create)
BK_DEFINE_EVENT(on_paint)
BK_DEFINE_EVENT(on_close)
BK_DEFINE_EVENT(on_resize)
BK_DEFINE_EVENT(mouse::on_enter)
BK_DEFINE_EVENT(mouse::on_exit)
BK_DEFINE_EVENT(mouse::on_move)
BK_DEFINE_EVENT(mouse::on_move_to)
BK_DEFINE_EVENT(mouse::on_mouse_down)
BK_DEFINE_EVENT(mouse::on_mouse_up)
BK_DEFINE_EVENT(mouse::on_mouse_wheel_v)
BK_DEFINE_EVENT(mouse::on_mouse_wheel_h)
BK_DEFINE_EVENT(keyboard::on_keydown)
BK_DEFINE_EVENT(keyboard::on_keyup)
BK_DEFINE_EVENT(bklib::ime_candidate_list::on_begin)
BK_DEFINE_EVENT(bklib::ime_candidate_list::on_update)
BK_DEFINE_EVENT(bklib::ime_candidate_list::on_end)

#undef BK_DEFINE_EVENT
