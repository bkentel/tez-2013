#include "pch.hpp"
#include "window.hpp"

#if defined(BOOST_OS_WINDOWS)
#   include "platform/window_windows.hpp"
#endif

using pw = bklib::platform_window;
using mouse = bklib::mouse;
using keyboard = bklib::keyboard;


////////////////////////////////////////////////////////////////////////////////
// bklib::mouse
////////////////////////////////////////////////////////////////////////////////
mouse::record
mouse::history(
    history_type const type
  , size_t       const n
) const
{
    BK_ASSERT(n < HISTORY_SIZE);
    BK_ASSERT(type == history_type::relative || type == history_type::absolute);

    return (type == history_type::relative)
        ? *(rel_history_.begin() + n)
        : *(abs_history_.begin() + n);
}
//--------------------------------------------------------------------------
void
mouse::push(
    history_type const type
  , record       const rec
) {
    if (type == history_type::relative) {
        rel_history_.push_front(rec);

        for (size_t i = 0; rec.has_buttons() && i < BUTTON_COUNT; ++i) {
            if (buttons_[i].state != rec.buttons[i]) {
                buttons_[i].state = rec.buttons[i];
                buttons_[i].time  = rec.time;
            }
        }
    } else if (type == history_type::absolute) {
        abs_history_.push_front(rec);

        x_ = rec.x;
        y_ = rec.y;
    }
}
//--------------------------------------------------------------------------
mouse::mouse()
  : x_{0}
  , y_{0}
{
    static auto const s = button_state::unknown;

    auto const now = clock::now();

    record const rec {
        now
        , 0, 0, 0
        , {{s, s, s, s, s}}
        , update_type::none
    };

    rel_history_.resize(HISTORY_SIZE, rec);
    abs_history_.resize(HISTORY_SIZE, rec);

    button_info const info = {button_state::unknown, now};
    std::fill_n(buttons_, BUTTON_COUNT, info);
}
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
