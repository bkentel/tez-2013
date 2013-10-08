#include "pch.hpp"
#include "window.hpp"

#if defined(BK_PLATFORM_WINDOWS)
#   include "platform/window_windows.hpp"
#endif

using pw = bklib::platform_window;

pw::~platform_window() {

}

pw::platform_window(
    platform_string title,
    unsigned width,
    unsigned height
)
: impl_ {new impl_t_ {*this}}
{
}

void pw::do_events() {
    impl_->do_events();
}

void pw::listen(pw::on_create callback) {
    impl_->listen(callback);
}

void pw::listen(pw::on_mouse_move_to callback) {
    impl_->listen(callback);
}

void pw::listen(bklib::ime_candidate_list::on_update callback) {
    impl_->listen(callback);
}
