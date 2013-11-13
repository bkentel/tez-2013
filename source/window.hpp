#pragma once

#include <memory>

#include <boost/circular_buffer.hpp>

#include "types.hpp"
#include "ime.hpp"
#include "callback.hpp"
#include "math.hpp"

namespace bklib {

//TODO move
template <typename T, size_t N = sizeof(T)>
struct print_size_of;

//TODO move
template <typename EnumType>
class bit_flags {
public:
    static_assert(std::is_enum<EnumType>::value, "must be an enum type.");
    using storage_type = std::underlying_type_t<EnumType>;

    bit_flags() BK_NOEXCEPT
        : value_{0}
    {
    }

    bit_flags(EnumType const value) BK_NOEXCEPT
        : value_{ static_cast<storage_type>(value) }
    {
    }

    inline bool operator&(EnumType const flag) const BK_NOEXCEPT {
        return (value_ & static_cast<storage_type>(flag)) != 0;
    }

    inline bit_flags operator|(EnumType const flag) const BK_NOEXCEPT {
        return { value_ | static_cast<storage_type>(flag) };
    }

    inline bit_flags operator|=(EnumType const flag) BK_NOEXCEPT {
        return (*this = *this | flag);
    }

    void reset() BK_NOEXCEPT { value_ = 0; }
    void reset(EnumType const value) BK_NOEXCEPT { value_ = static_cast<storage_type>(value); }
private:
    bit_flags(storage_type value) BK_NOEXCEPT : value_{value} {}
    storage_type value_;
};

//==============================================================================
//! Keyboard keys; roughly aligned with ASCII codes.
//==============================================================================
enum class keys : uint8_t {
    NONE
  , K0 = '0', K1, K2, K3, K4, K5, K6, K7, K8, K9
  , A  = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
  , NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9
  , NUM_DIV, NUM_MUL, NUM_MIN, NUM_ADD, NUM_DEC, NUM_ENTER, NUM_LCK
  , F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24
  , LEFT, RIGHT, UP, DOWN
  , CTRL_L, CTRL_R
  , ALT_L, ALT_R
  , SHIFT_L, SHIFT_R
  , ENTER
  , INS, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
};

//==============================================================================
//! The state of the keyboard.
//==============================================================================
class keyboard {
public:
    using clock      = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using key        = bklib::keys;

    struct record {
        time_point time;    //!<< Time stamp of when the keystate changed.
        bool       is_down; //!<< The key is down.

        explicit operator bool() const BK_NOEXCEPT { return is_down; }
    };

    record operator[](key const k) const {
        auto const i = key_to_index_(k);
        return state_[i];
    }
    
    bool set_state(key const k, bool const is_down) {
        auto const i = key_to_index_(k);

        if (state_[i].is_down != is_down) {
            state_[i].is_down = is_down;
            state_[i].time    = clock::now();

            return false;
        }

        return true;
    }

    BK_DECLARE_EVENT(on_keydown,   void (keyboard& state, key k));
    BK_DECLARE_EVENT(on_keyup,     void (keyboard& state, key k));
    BK_DECLARE_EVENT(on_keyrepeat, void (keyboard& state, key k));
private:
    static size_t key_to_index_(key const k) BK_NOEXCEPT {
        return static_cast<std::underlying_type_t<key>>(k);
    }

    std::array<record, 0xFF> state_;
};

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

    enum class update_type : uint8_t {
        none              = 0      //!<< No update.
      , absolute_position = 1 << 0 //!<< The absolute position has changed.
      , relative_position = 1 << 1 //!<< The relativeposition has changed.
      , button            = 1 << 2 //!<< A button's state has changed.
      , wheel_vertical    = 1 << 3 //!<< The vertical mouse wheel was scrolled.
      , wheel_horizontal  = 1 << 4 //!<< The horizontal mouse wheel was scrolled.
      , alt_down          = 1 << 5 //!<< The alt was down at the time of the update.
      , ctrl_down         = 1 << 6 //!<< The ctrl was down at the time of the update.
      , shift_down        = 1 << 7 //!<< The shift was down at the time of the update.
    };

    enum class button_state : uint8_t {
        unknown   //!<< The button state is unknown.
      , is_up     //!<< The button is up.
      , went_up   //!<< The button just went up.
      , is_down   //!<< The button is down.
      , went_down //!<< The button just went down.
    };

    enum class history_type {
        relative //!<< Movement is relative to the last position.
      , absolute //!<< Absolute coordinates relative to the window's client area.
    };

    //! History record.
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

    int x_, y_;
    button_info buttons_[BUTTON_COUNT];
};

//==============================================================================
//! Abstraction of a native window.
//==============================================================================
class platform_window {
public:
    struct platform_handle;

    class impl_t_;
    friend impl_t_;

    enum class state {
        starting, running, finished_error, finished_ok
    };

    ~platform_window();
    explicit platform_window(platform_string title);

    BK_DECLARE_EVENT(on_create, void());
    BK_DECLARE_EVENT(on_paint,  void());
    BK_DECLARE_EVENT(on_close,  void());
    BK_DECLARE_EVENT(on_resize, void(unsigned w, unsigned h));

    void listen(on_create callback);
    void listen(on_paint  callback);
    void listen(on_close  callback);
    void listen(on_resize callback);

    void listen(mouse::on_enter   callback);
    void listen(mouse::on_exit    callback);

    void listen(mouse::on_move          callback);
    void listen(mouse::on_move_to       callback);
    void listen(mouse::on_mouse_down    callback);
    void listen(mouse::on_mouse_up      callback);
    void listen(mouse::on_mouse_wheel_v callback);
    void listen(mouse::on_mouse_wheel_h callback);

    void listen(keyboard::on_keydown callback);
    void listen(keyboard::on_keyup   callback);

    void listen(ime_candidate_list::on_begin  callback);
    void listen(ime_candidate_list::on_update callback);
    void listen(ime_candidate_list::on_end    callback);

    bool is_running() const;

    std::future<int> result_value();

    void do_events();

    platform_handle get_handle() const;
private:
    std::unique_ptr<impl_t_> impl_;
};

#if defined(BOOST_OS_WINDOWS)
struct platform_window::platform_handle {
    operator HWND() const { return value; }
    HWND value;
};
#endif

} //namespace bklib
