#include "pch.hpp"
#include "mouse.hpp"

using mouse = ::bklib::mouse;

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
mouse::mouse() {
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
