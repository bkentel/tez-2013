#pragma once

#include <chrono>
#include <array>

#include <boost/circular_buffer.hpp>

#include "types.hpp"
#include "callback.hpp"
#include "util.hpp"
#include "math.hpp"

namespace bklib {
//==============================================================================
//! The history and current state of the mouse.
//==============================================================================
class mouse {
public:
    using clock = std::chrono::high_resolution_clock;

    static size_t const BUTTON_COUNT = 5;   //!<< Number of mouse buttons.
    static size_t const HISTORY_SIZE = 100; //!<< Size of the history.

    BK_DECLARE_EVENT(on_enter, void (mouse& m));
    BK_DECLARE_EVENT(on_exit,  void (mouse& m));

    BK_DECLARE_EVENT(on_hover,     void (mouse& m));
    BK_DECLARE_EVENT(on_click,     void (mouse& m, unsigned button));
    BK_DECLARE_EVENT(on_dbl_click, void (mouse& m, unsigned button));

    BK_DECLARE_EVENT(on_move,          void (mouse& m, int x, int y));
    BK_DECLARE_EVENT(on_move_to,       void (mouse& m, int dx, int dy));
    BK_DECLARE_EVENT(on_mouse_down,    void (mouse& m, unsigned button));
    BK_DECLARE_EVENT(on_mouse_up,      void (mouse& m, unsigned button));
    BK_DECLARE_EVENT(on_mouse_wheel_v, void (mouse& m, int delta));
    BK_DECLARE_EVENT(on_mouse_wheel_h, void (mouse& m, int delta));
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    enum class update_type : uint8_t {
        none              //!<< No update.
      , absolute_position //!<< The absolute position has changed.
      , relative_position //!<< The relativeposition has changed.
      , button            //!<< A button's state has changed.
      , wheel_vertical    //!<< The vertical mouse wheel was scrolled.
      , wheel_horizontal  //!<< The horizontal mouse wheel was scrolled.
      , alt_down          //!<< The alt was down at the time of the update.
      , ctrl_down         //!<< The ctrl was down at the time of the update.
      , shift_down        //!<< The shift was down at the time of the update.
    };
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    enum class button_state : uint8_t {
        unknown   //!<< The button state is unknown.
      , is_up     //!<< The button is up.
      , went_up   //!<< The button just went up.
      , is_down   //!<< The button is down.
      , went_down //!<< The button just went down.
    };
    //--------------------------------------------------------------------------
    //!
    //--------------------------------------------------------------------------
    enum class history_type {
        relative //!<< Movement is relative to the last position.
      , absolute //!<< Absolute coordinates relative to the window's client area.
    };
    //--------------------------------------------------------------------------
    //! History record.
    //--------------------------------------------------------------------------
    struct record {
        //! The time stamp for the event.
        clock::time_point time;

        //! The absolute or relative x position of the mouse based on @c flags.
        int16_t x;

        //! The absolute or relative y position of the mouse based on @c flags.
        int16_t y;

        //! If (flags & wheel_vertical) or (flags & wheel_vertical), gives the
        //! mouse wheel delta.
        int16_t wheel_delta; 

        //! The state of the buttons (cumulative).
        std::array<button_state, BUTTON_COUNT> buttons;

        //! Update flags for this update.
        bit_flags<update_type> flags;

        bool has_relative() const BK_NOEXCEPT { return flags & update_type::relative_position; }
        bool has_absolute() const BK_NOEXCEPT { return flags & update_type::absolute_position; }
        bool has_buttons()  const BK_NOEXCEPT { return flags & update_type::button; }
    };
    //--------------------------------------------------------------------------
    //! Button info.
    //--------------------------------------------------------------------------
    struct button_info {
        button_state      state; //!<< Button state.
        clock::time_point time;  //!<< Time stamp of the last change.

        //! True if the button is or just went down. False otherwise.
        explicit operator bool() const BK_NOEXCEPT {
            return state == button_state::is_down
                || state == button_state::went_down;
        }
    };
    //--------------------------------------------------------------------------
    //! Get the nth most recent update.
    //! @pre n < HISTORY_SIZE.
    //--------------------------------------------------------------------------
    record history(history_type type, size_t n = 0) const;
    //--------------------------------------------------------------------------
    //! Add a record to the history.
    //--------------------------------------------------------------------------
    void push(history_type type, record rec);

    mouse();

    point2d<int> position() const BK_NOEXCEPT {
        return {x_, y_};
    }
    
    button_info button(size_t const n) const BK_NOEXCEPT {
        BK_ASSERT(n < BUTTON_COUNT);
        return buttons_[n];
    }
private:
    boost::circular_buffer<record> rel_history_;
    boost::circular_buffer<record> abs_history_;

    int x_ = 0;
    int y_ = 0;
    button_info buttons_[BUTTON_COUNT];
};

} //namespace bklib
