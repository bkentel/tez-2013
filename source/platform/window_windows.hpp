#pragma once

#include "platform.hpp"
#include "com.hpp"
#include "concurrent_queue.hpp"
#include "window.hpp"
#include "ime_windows.hpp"
#include "exception.hpp"

namespace bklib {

template <typename T>
struct move_on_copy {
    move_on_copy(T&& value)
      : value{std::move(value)}
    {
    }

    move_on_copy(move_on_copy&& other)
      : value{std::move(other.value)}
    {
    }

    move_on_copy& operator=(move_on_copy&& rhs) {
        swap(rhs);
        return *this;
    }

    move_on_copy(move_on_copy const& other)
      : move_on_copy(std::move(other.value))
    {
    }

    move_on_copy& operator=(move_on_copy const& rhs) {
        return (*this = std::move(rhs));
    }

    T const* operator->() const { return &value; }
    T* operator->() { return &value; }

    void swap(move_on_copy& other) {
        using std::swap;
        swap(value, other.value);
    }

    mutable T value;
};

template <typename T>
void swap(move_on_copy<T>& lhs, move_on_copy<T>& rhs) {
    lhs.swap(rhs);
}

struct hwnd_deleter {
    typedef HWND pointer;
    void operator()(pointer p) const BK_NOEXCEPT {
        ::DestroyWindow(p);
    }
};

using window_handle = std::unique_ptr<HWND, hwnd_deleter>;

//==============================================================================
//!
//==============================================================================
class platform_window::impl_t_ {
    impl_t_(impl_t_ const&) = delete;
    impl_t_& operator=(impl_t_ const&) = delete;
public:
    using invocable = std::function<void()>;

    impl_t_();

    void shutdown();

    HWND handle() const BK_NOEXCEPT { return window_.get(); }

    bool is_running() const BK_NOEXCEPT { return state_ == state::running; }

    std::future<int> result_value() {
        return result_.get_future();
    }

    void do_events();

    platform_window::platform_handle get_handle() const;

    void listen(on_create callback);
    void listen(on_paint  callback);
    void listen(on_close  callback);
    void listen(on_resize callback);

    void listen(mouse::on_enter         callback);
    void listen(mouse::on_exit          callback);
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
private:
    window_handle window_;

    mouse    mouse_state_;
    keyboard keyboard_state_;

    on_create on_create_;
    on_paint  on_paint_;
    on_close  on_close_;
    on_resize on_resize_;

    mouse::on_move_to       on_mouse_move_to_;
    mouse::on_move          on_mouse_move_;
    mouse::on_mouse_down    on_mouse_down_;
    mouse::on_mouse_up      on_mouse_up_;
    mouse::on_mouse_wheel_v on_mouse_wheel_v_;
    mouse::on_mouse_wheel_h on_mouse_wheel_h_;

    keyboard::on_keydown   on_keydown_;
    keyboard::on_keyup     on_keyup_;
    keyboard::on_keyrepeat on_keyrepeat_;

    LRESULT local_wnd_proc_(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
    static LRESULT CALLBACK wnd_proc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static HWND create_window_(impl_t_* win);

    static void push_job_(invocable job);
    static void push_event_(invocable event);

    static void main_();
    static void init_();

    static concurrent_queue<invocable> work_queue_;
    static concurrent_queue<invocable> event_queue_;

    static DWORD thread_id_;
    static state state_;
    static std::promise<int> result_;
};

namespace detail {

class raw_input {
public:
    explicit raw_input(LPARAM lParam);
    raw_input(const raw_input& other) = delete;
    raw_input& operator=(const raw_input& rhs) = delete;

    raw_input(raw_input&& other)
      : buffer_size_{other.buffer_size_}
      , buffer_{std::move(other.buffer_)}
    {
    }

    raw_input& operator=(raw_input&& rhs) {
        swap(rhs);
        return *this;
    }

    void swap(raw_input& other) {
        using std::swap;
        swap(buffer_size_, other.buffer_size_);
        swap(buffer_,      other.buffer_);
    }

    bool is_mouse()    const { return get_().header.dwType == RIM_TYPEMOUSE; }
    bool is_keyboard() const { return get_().header.dwType == RIM_TYPEKEYBOARD; }
    bool is_hid()      const { return get_().header.dwType == RIM_TYPEHID; }
    
    RAWKEYBOARD const& keyboard() const {
        BK_ASSERT(is_keyboard());
        return get_().data.keyboard;
    }

    RAWMOUSE const& mouse() const {
        BK_ASSERT(is_mouse());
        return get_().data.mouse;
    }

    void handle_message();

    enum class mouse_button {
        no_change, went_down, went_up
    };

    mouse_button button_state(unsigned button) const {
        BK_ASSERT(button < 6);

        auto const flags = mouse().usButtonFlags;
        auto const mask  = 3 << 2*button;
        auto const shift = 2*button;

        auto const result = (flags & mask) >> shift;
        BK_ASSERT(result < 3);

        return static_cast<mouse_button>(result);
    }

    struct key_info {
        USHORT scancode;
        keys   key;
        bool   went_down;
        bool   discard;
    };

    key_info get_key_info() const {
        auto const& kb = keyboard();

        if (kb.VKey == 0xFF) {
            return {0, keys::NONE, false, true};
        }

        bool const went_down = !(kb.Flags & RI_KEY_BREAK);
        bool const is_e0     =  (kb.Flags & RI_KEY_E0) != 0;
        bool const is_e1     =  (kb.Flags & RI_KEY_E1) != 0;

        auto const scancode = [&] {
            // pause is a special case... bug in the API
            if (kb.VKey == VK_PAUSE) {
                return static_cast<USHORT>(0x45);
            // numlock is another special case
            } else if (kb.VKey == VK_NUMLOCK || kb.VKey == VK_LWIN || kb.VKey == VK_RWIN || kb.VKey == VK_APPS) {
                return static_cast<USHORT>(::MapVirtualKeyW(kb.VKey, MAPVK_VK_TO_VSC) | 0x100);
            } else if (kb.VKey == VK_SNAPSHOT) {
                return static_cast<USHORT>(::MapVirtualKeyW(kb.VKey, MAPVK_VK_TO_VSC));
            } else {
                return kb.MakeCode;
            }
        }();
       
        // virtual key code
        auto const vkey = [&] {
            UINT const flag = is_e0 ? (0xE0 << 8) :
                              is_e1 ? (0xE1 << 8) : 0;

            auto const key = ::MapVirtualKeyExW(
                scancode | flag, MAPVK_VSC_TO_VK_EX, 0
            );

            return key ? key : kb.VKey;
        }();

        bklib::keys key = [&] {
            if (vkey >= '0' && vkey <= '9') {
                return static_cast<bklib::keys>(vkey);
            } else if (vkey >= 'A' && vkey <= 'Z') {
                return static_cast<bklib::keys>(vkey);
            } else if (!is_e0 && vkey >= VK_NUMPAD0 && vkey <= VK_NUMPAD9) {
                auto const x = vkey - VK_NUMPAD0;
                return static_cast<bklib::keys>(static_cast<int>(keys::NUM_0) + x);
            } else if (vkey >= VK_F1 && vkey <= VK_F24) {
                auto const x = vkey - VK_F1;
                return static_cast<bklib::keys>(static_cast<int>(keys::F1) + x);
            }

            switch (vkey) {
            case VK_LCONTROL : return keys::CTRL_L;
            case VK_RCONTROL : return keys::CTRL_R;
            case VK_LMENU    : return keys::ALT_L;
            case VK_RMENU    : return keys::ALT_R;

            case VK_CONTROL  : return is_e0 ? keys::CTRL_R    : keys::CTRL_L;
            case VK_MENU     : return is_e0 ? keys::ALT_R     : keys::ALT_L;
            case VK_SHIFT    : return is_e0 ? keys::SHIFT_R   : keys::SHIFT_L;
            case VK_RETURN   : return is_e0 ? keys::NUM_ENTER : keys::ENTER;
            case VK_NUMPAD2  : BK_ASSERT(is_e0); return keys::DOWN;
            case VK_NUMPAD4  : BK_ASSERT(is_e0); return keys::LEFT;
            case VK_NUMPAD6  : BK_ASSERT(is_e0); return keys::RIGHT;
            case VK_NUMPAD8  : BK_ASSERT(is_e0); return keys::UP;
            case VK_INSERT   : return is_e0 ? keys::INS : keys::NUM_0;
            case VK_DELETE   : return is_e0 ? keys::DEL : keys::NUM_DEC;
            case VK_HOME     : return is_e0 ? keys::HOME : keys::NUM_7;
            case VK_END      : return is_e0 ? keys::END : keys::NUM_1;
            case VK_PRIOR    : return is_e0 ? keys::PAGE_UP : keys::NUM_9;
            case VK_NEXT     : return is_e0 ? keys::PAGE_DOWN : keys::NUM_3;
            case VK_LEFT     : return is_e0 ? keys::LEFT  : keys::NUM_4;
            case VK_RIGHT    : return is_e0 ? keys::RIGHT : keys::NUM_6;
            case VK_UP       : return is_e0 ? keys::UP    : keys::NUM_8;
            case VK_DOWN     : return is_e0 ? keys::DOWN  : keys::NUM_2;
            case VK_CLEAR    : BK_ASSERT(!is_e0); return keys::NUM_5;
            }

            return keys::NONE;
        }();

        // set the extended bit
//        auto const final_scancode = is_e0 ? scancode | 0x100 : scancode;

        return {scancode, key, went_down, false};
    }

    static std::wstring const& get_key_name(UINT scancode) {
        BK_ASSERT(scancode < key_names_.size());
        return key_names_[scancode];
    }

    static void init_key_names() {
        BK_ASSERT(key_names_.empty());

        static size_t const BUF_SIZE  = 64;
        static UINT   const KEY_COUNT = 0x200;

        wchar_t name_buffer[BUF_SIZE];

        key_names_.reserve(KEY_COUNT);

        for (UINT scancode = 0; scancode < KEY_COUNT; ++scancode) {
            auto const length = ::GetKeyNameTextW(
                (scancode << 16), name_buffer, BUF_SIZE
            );

            key_names_.emplace_back(name_buffer, name_buffer + length);
        }
    }
private:
    RAWINPUT const& get_() const {
        return *reinterpret_cast<RAWINPUT*>(buffer_.get());
    }

    size_t                  buffer_size_;
    std::unique_ptr<char[]> buffer_;

    static std::vector<std::wstring> key_names_;
};


} //namespace detail


} //namespace bklib