#include "pch.hpp"
#include "window.hpp"

#if defined(BK_PLATFORM_WINDOWS)
#   include "platform/window_windows.hpp"
#endif

using pw = bklib::platform_window;
using mouse = bklib::mouse;

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

void pw::listen(on_create callback) {
    impl_->listen(callback);
}
void pw::listen(on_close  callback) {
    impl_->listen(callback);
}
void pw::listen(on_resize callback) {
    impl_->listen(callback);
}
void pw::listen(mouse::on_enter callback) {
    impl_->listen(callback);
}
void pw::listen(mouse::on_exit callback) {
    impl_->listen(callback);
}
void pw::listen(mouse::on_move callback) {
    impl_->listen(callback);
}
void pw::listen(mouse::on_move_to callback) {
    impl_->listen(callback);
}
void pw::listen(bklib::ime_candidate_list::on_begin callback) {
    impl_->listen(callback);
}
void pw::listen(bklib::ime_candidate_list::on_update callback) {
    impl_->listen(callback);
}
void pw::listen(bklib::ime_candidate_list::on_end callback) {
    impl_->listen(callback);
}
