#pragma once

#include <memory>

//#define BOOST_CB_DISABLE_DEBUG
#include <boost/circular_buffer.hpp>

#include "types.hpp"
#include "ime.hpp"
#include "callback.hpp"

namespace bklib {

template <typename T, size_t N = sizeof(T)>
struct print_size_of;

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

enum class keys : uint8_t {
    NONE
  , K0 = '0', K1, K2, K3, K4, K5, K6, K7, K8, K9
  , A  = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
  , NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9
  , NUM_DIV, NUM_MUL, NUM_MIN, NUM_ADD, NUM_DEC, NUM_ENTER, NUM_LCK
  , F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24
  , LEFT, RIGHT, UP, DOWN
  , CTRL_L,  CTRL_R
  , ALT_L,   ALT_R
  , SHIFT_L, SHIFT_R
  , ENTER
  , INS, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
};

class keyboard {
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using key = bklib::keys;

    struct record {
        time_point time;
        bool       is_down;
    };

    record operator[](key k) const {
        auto const i = key_to_index_(k);
        return state_[i];
    }
    
    bool set_state(key k, bool is_down) {
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

class mouse {
public:
    using clock = std::chrono::high_resolution_clock;

    BK_DECLARE_EVENT(on_enter, void (mouse& m));
    BK_DECLARE_EVENT(on_exit,  void (mouse& m));

    BK_DECLARE_EVENT(on_move,    void (mouse& m, int x, int y));
    BK_DECLARE_EVENT(on_move_to, void (mouse& m, int dx, int dy));

    BK_DECLARE_EVENT(on_mouse_down,    void (mouse& m, unsigned button));
    BK_DECLARE_EVENT(on_mouse_up,    void (mouse& m, unsigned button));

    BK_DECLARE_EVENT(on_mouse_wheel_v,    void (mouse& m, int delta));
    BK_DECLARE_EVENT(on_mouse_wheel_h,    void (mouse& m));

    enum class update_type : char {
        none              = 0
      , absolute_position = 1 << 0
      , relative_position = 1 << 1
      , button            = 1 << 2
      , wheel_vertical    = 1 << 3
      , wheel_horizontal  = 1 << 4
    };

    enum class button_state : char {
        unknown, is_up, went_up, is_down, went_down
    };

    struct record {
        clock::time_point           time;
        std::int32_t                x;
        std::int32_t                y;
        std::array<button_state, 5> buttons;
        bit_flags<update_type>      flags;
        std::int16_t                wheel_delta;
    };

    record history(size_t n = 0) const {
        auto const size = history_.size();
        BK_ASSERT(n < size);

        auto it = history_.begin() + n;
        record result = *it;
        return result;
    }

    void push(record rec) {
        history_.push_front(rec);
    }

    mouse() {
        static auto const state = button_state::unknown;

        record const rec {
            clock::now()
          , 0, 0
          , {{state, state, state, state, state}}
          , update_type::none
          , 0
        };

        history_.set_capacity(100);
        push(rec);
    }
private:
    boost::circular_buffer<record> history_;
    clock::time_point buttons_[5];
};

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
    void listen(mouse::on_move    callback);
    void listen(mouse::on_move_to callback);

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
