#include "pch.hpp"

#include "window_windows.hpp"
#include "ime_windows.hpp"
#include "com.hpp"

#pragma comment(lib, "Imm32.lib")

#define BK_WM_ASSOCIATE_TSF (WM_APP + 1)

using window = bklib::platform_window::impl_t_;

//------------------------------------------------------------------------------
bklib::concurrent_queue<window::invocable> window::work_queue_;
bklib::concurrent_queue<window::invocable> window::event_queue_;
DWORD                               window::thread_id_ {0};
bklib::platform_window::state window::state_ = bklib::platform_window::state::starting;
std::promise<int> window::result_;

std::vector<std::wstring> bklib::detail::raw_input::key_names_;
//------------------------------------------------------------------------------

namespace {
    //--------------------------------------------------------------------------
    std::once_flag once_flag; //used for initialization
    //--------------------------------------------------------------------------
    template <typename T>
    T* get_user_data(HWND hWnd) {
        ::SetLastError(0);
        auto const result = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

        if (result == 0) {
            auto const e = ::GetLastError();
            if (e != 0) {
                BOOST_THROW_EXCEPTION(bklib::win::windows_error {}
                    << boost::errinfo_api_function("GetWindowLongPtrW")
                    << boost::errinfo_errno(e)
                );
            }
        }

        return reinterpret_cast<T*>(result);
    }
    //--------------------------------------------------------------------------
    template <typename T>
    LONG set_user_data(HWND hWnd, T* data) {
        ::SetLastError(0);
        auto const value = reinterpret_cast<LONG>(data);
        auto const result = ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, value);

        if (result == 0) {
            auto const e = ::GetLastError();
            if (e != 0) {
                BOOST_THROW_EXCEPTION(bklib::win::windows_error {}
                    << boost::errinfo_api_function("SetWindowLongPtrW")
                    << boost::errinfo_errno(e)
                );
            }
        }

        return result;
    }
} //namespace
//------------------------------------------------------------------------------
void window::push_job_(invocable job) {
    work_queue_.push(std::move(job));

    auto const result = ::PostThreadMessageW(thread_id_, WM_NULL, 0, 0);
    if (result == 0) {
        BOOST_THROW_EXCEPTION(bklib::win::windows_error {}
            << boost::errinfo_api_function("PostThreadMessageW")
            << boost::errinfo_errno(::GetLastError())
        );
    }
}
//------------------------------------------------------------------------------
void window::push_event_(invocable event) {
    event_queue_.push(std::move(event));
}
//------------------------------------------------------------------------------

namespace {
    void init_com() {
        BK_THROW_ON_COM_FAIL(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

        bklib::win::com_ptr<IGlobalOptions> options {[] {
            IGlobalOptions* result = nullptr;

            BK_THROW_ON_COM_FAIL(::CoCreateInstance(CLSID_GlobalOptions,
                nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&result))
            );

            return result;
        }()};

        BK_THROW_ON_COM_FAIL(
            options->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY)
        );
    }

    template <typename T, size_t N>
    size_t elements_in(T const (&)[N]) {
        return N;
    }

    void init_raw_input() {
        static RAWINPUTDEVICE const devices[] = {
            {0x01, 0x02, 0, 0} //mouse
          , {0x01, 0x06, 0, 0} //keyboard
        };

        auto const result = ::RegisterRawInputDevices(
            devices
          , elements_in(devices)
          , sizeof(RAWINPUTDEVICE)
        );

        if (result == FALSE) {
            BK_DEBUG_BREAK(); //TODO
        }

        bklib::detail::raw_input::init_key_names();
    }

} //namespace

void window::init_() {
    std::call_once(once_flag, [] {
        ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

        init_com();
        init_raw_input();

        auto const result = ::ImmDisableIME(static_cast<DWORD>(-1));
        if (result == FALSE) {
            BK_THROW_WINAPI(ImmDisableIME);
        }

        std::thread window_thread(window::main_);
        thread_id_ = ::GetThreadId(window_thread.native_handle());
        window_thread.detach();

        while ((state_ == state::starting) || thread_id_ == 0) {}
    });
}
//------------------------------------------------------------------------------
void window::main_()
try {
    BK_ASSERT(state_ == state::starting);

    MSG msg {0};
    init_com();

    //enusure the thread has a message queue before continuing
    ::PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    //ime_manager_ = std::make_unique<impl::ime_manager>(); //TODO

    state_ = state::running;
    BK_SCOPE_EXIT_NAME(on_scope_exit, {state_ = state::finished_error;});

    for (;;) {
        auto const result = ::GetMessageW(&msg, 0, 0, 0);

        while (!work_queue_.is_empty()) {
            work_queue_.pop()();
        }

        if (result == TRUE) {
            //OK
            if (msg.message == BK_WM_ASSOCIATE_TSF) {
                //ime_manager_->associate(msg.hwnd); //TODO
            }

            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        } else if (result == FALSE) {
            //WM_QUIT
            break;
        } else {
            //Error
            return;
        }
    }

    on_scope_exit.cancel();
    state_ = state::finished_ok;

    result_.set_value_at_thread_exit(0);
} catch (...) {
    result_.set_exception_at_thread_exit(std::current_exception());
}

//------------------------------------------------------------------------------
HWND window::create_window_(window* win) {
    static wchar_t const CLASS_NAME[] = L"bkwin";

    init_();

    std::promise<HWND> promise_window;
    std::future<HWND>  future_window = promise_window.get_future();

    push_job_([&] {
        auto const instance = ::GetModuleHandleW(nullptr);

        WNDCLASSEXW const wc = {
            sizeof(WNDCLASSEXW),
            CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
            window::wnd_proc_,
            0,
            0,
            instance,
            ::LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_WINLOGO)),
            ::LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW)),
            nullptr,
            nullptr,
            CLASS_NAME,
            nullptr
        };

        ::RegisterClassExW(&wc); // ignore return value

        auto const result = ::CreateWindowExW(
            0,
            CLASS_NAME,
            L"window",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            (HWND)nullptr,
            (HMENU)nullptr,
            instance,
            win
        );

        if (result == nullptr) {
            BK_THROW_WINAPI(CreateWindowExW);
        }

        ::PostMessageW(result, BK_WM_ASSOCIATE_TSF, reinterpret_cast<WPARAM>(result), 0);

        promise_window.set_value(result);
    });

    return future_window.get();
}
//------------------------------------------------------------------------------
LRESULT CALLBACK window::wnd_proc_(HWND hWnd, UINT const uMsg, WPARAM wParam, LPARAM lParam)
try {
    if (uMsg == WM_NCCREATE) {
        auto const create_struct = reinterpret_cast<LPCREATESTRUCTW const>(lParam);
        auto win = reinterpret_cast<UNALIGNED window*>(create_struct->lpCreateParams);

        auto const old = set_user_data(hWnd, win);
        BK_ASSERT(old == 0);

        BK_ASSERT(win->window_ == nullptr);
        win->window_.reset(hWnd);
    }

    auto const win = get_user_data<window>(hWnd);
    if (win == nullptr) {
        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    return win->local_wnd_proc_(uMsg, wParam, lParam);
} catch (std::exception& e) {
    BK_UNUSED(e);

    BK_DEBUG_BREAK();
    ::PostQuitMessage(-1);
    return 0;
} catch (...) {
    BK_DEBUG_BREAK();
    ::PostQuitMessage(-1);
    return 0;
}
//------------------------------------------------------------------------------
namespace {

bklib::mouse::record
get_mouse_record(
    bklib::detail::raw_input const& input
  , bklib::mouse::record            prev_record
) {
    using flags = bklib::mouse::update_type;
    using state = bklib::mouse::button_state;
    using mb    = bklib::detail::raw_input::mouse_button;

    auto const& m = input.mouse();

    prev_record.flags.reset();

    if (m.lLastX || m.lLastY) {
        prev_record.flags |= flags::relative_position;
        prev_record.x = static_cast<int16_t>(m.lLastX);
        prev_record.y = static_cast<int16_t>(m.lLastY);
    } else {
        prev_record.x = 0;
        prev_record.y = 0;
    }

    if (m.ulButtons & (RI_MOUSE_WHEEL - 1)) {
        prev_record.flags |= flags::button;
    }

    for (size_t i = 0; i < 5; ++i) {
        auto& b = prev_record.buttons[i];

        switch (input.button_state(i)) {
        case mb::went_down : b = state::went_down; break;
        case mb::went_up :   b = state::went_up;   break;
        case mb::no_change :
            if      (b == state::went_down) { b = state::is_down; }
            else if (b == state::went_up)   { b = state::is_up; }
            break;           
        }
    }

    if (m.ulButtons & RI_MOUSE_WHEEL) {
        prev_record.flags |= flags::wheel_vertical;
        prev_record.wheel_delta = static_cast<SHORT>(m.usButtonData);
    }

    prev_record.time = bklib::mouse::clock::now();

    return prev_record;
}

} //namespace

//------------------------------------------------------------------------------
namespace {
static int translate_key(bklib::keys const key) BK_NOEXCEPT {
    switch (key) {
    case bklib::keys::K0 : return '0';
    case bklib::keys::K1 : return '1';
    case bklib::keys::K2 : return '2';
    case bklib::keys::K3 : return '3';
    case bklib::keys::K4 : return '4';
    case bklib::keys::K5 : return '5';
    case bklib::keys::K6 : return '6';
    case bklib::keys::K7 : return '7';
    case bklib::keys::K8 : return '8';
    case bklib::keys::K9 : return '9';
    }

    return 0;
}
} //namespace

//------------------------------------------------------------------------------
LRESULT window::local_wnd_proc_(
    UINT   const uMsg
  , WPARAM const wParam
  , LPARAM const lParam
) {
    //--------------------------------------------------------------------------
    // WM_INPUT
    //--------------------------------------------------------------------------
    auto const handle_input = [&]() -> LRESULT {
        using flags   = bklib::mouse::update_type;
        using history = bklib::mouse::history_type;

        auto input = move_on_copy<detail::raw_input>(detail::raw_input{lParam});

        if (input->is_mouse()) {
            push_event_([=] {
                auto prev_rec = mouse_state_.history(history::relative);
                auto record   = get_mouse_record(input.value, prev_rec);

                if (!!keyboard_state_[keys::ALT_L]
                 || !!keyboard_state_[keys::ALT_L]
                ) {
                   record.flags |= flags::alt_down; 
                }

                if (!!keyboard_state_[keys::CTRL_L]
                 || !!keyboard_state_[keys::CTRL_R]
                ) {
                   record.flags |= flags::ctrl_down; 
                }

                if (!!keyboard_state_[keys::SHIFT_L]
                 || !!keyboard_state_[keys::SHIFT_R]
                ) {
                   record.flags |= flags::shift_down; 
                }
                
                mouse_state_.push(history::relative, record);

                bool const is_move    = record.flags & flags::relative_position;
                bool const is_wheel_v = record.flags & flags::wheel_vertical;

                if (is_move && on_mouse_move_) {
                    on_mouse_move_(mouse_state_, record.x, record.y);
                }

                if (is_wheel_v && on_mouse_wheel_v_) {
                    on_mouse_wheel_v_(mouse_state_, record.wheel_delta);
                }
            });
        } else if (input->is_keyboard()) {
            push_event_([=] {
                auto const info = input->get_key_info();
                if (info.discard) return;

                auto const repeat = keyboard_state_.set_state(
                    info.key, info.went_down
                );

                if (repeat) {
                    if (on_keyrepeat_) on_keyrepeat_(keyboard_state_, info.key);
                } else if (info.went_down && on_keydown_) {
                    on_keydown_(keyboard_state_, info.key);
                } else if (!info.went_down && on_keyup_) {
                    on_keyup_(keyboard_state_, info.key);
                }
            });
        }
        
        input->handle_message();
        return 0;
    };
    //--------------------------------------------------------------------------
    // WM_PAINT
    //--------------------------------------------------------------------------
    auto const handle_paint = [&]() -> LRESULT {
        push_event_([this] {
            if (on_paint_) on_paint_();
        });

        return 0;
    };
    //--------------------------------------------------------------------------
    // WM_MOUSEMOVE
    //--------------------------------------------------------------------------
    auto const handle_mouse_move = [&]() -> LRESULT {
        using flags   = bklib::mouse::update_type;
        using history = bklib::mouse::history_type;

        auto const x    = static_cast<int16_t>(lParam & 0xFFFF);
        auto const y    = static_cast<int16_t>((lParam >> 16) & 0xFFFF);
        auto const time = bklib::mouse::clock::now();

        push_event_([=] {
            auto record = mouse_state_.history(history::absolute);

            auto const old_flags = record.flags;
            record.flags.reset(flags::absolute_position);

            if (old_flags & flags::alt_down)   record.flags |= flags::alt_down;
            if (old_flags & flags::ctrl_down)  record.flags |= flags::ctrl_down;
            if (old_flags & flags::shift_down) record.flags |= flags::shift_down;

            record.x    = x;
            record.y    = y;
            record.time = time;

            mouse_state_.push(history::absolute, record);

            if (on_mouse_move_to_) {
                on_mouse_move_to_(mouse_state_, record.x, record.y);
            }
        });

        return 0;
    };
    //--------------------------------------------------------------------------
    // WM_SIZE
    //--------------------------------------------------------------------------
    auto const handle_size = [&]() -> LRESULT {
        push_event_([this] {
            RECT r {};
            ::GetClientRect(handle(), &r);
            
            if (on_resize_) on_resize_(
                r.right  - r.left
              , r.bottom - r.top
            );
        });

        return 0;
    };
    //--------------------------------------------------------------------------

    switch (uMsg) {
    default :
        break;
    case WM_MOUSEHWHEEL:
    case WM_MOUSEWHEEL :
        break;
    case WM_INPUT : 
        return handle_input();
        break;
    case WM_PAINT :
        return handle_paint();
        break;
    case WM_ERASEBKGND :
        return 1;
        break;
    case WM_DESTROY :
        ::PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        break;
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
        break;
    case WM_MOUSEMOVE :
        return handle_mouse_move();
        break;
    case WM_SIZE:
        return handle_size();
        break;
    }

    return ::DefWindowProcW(window_.get(), uMsg, wParam, lParam);
}
//------------------------------------------------------------------------------
window::impl_t_()
    : window_ {nullptr}
{
    auto const result = create_window_(this);
    BK_ASSERT(result == window_.get());

    ::UpdateWindow(window_.get());
    ::ShowWindow(window_.get(), SW_SHOWDEFAULT);
}
//------------------------------------------------------------------------------
void window::do_events() {
    while (!event_queue_.is_empty()) {
        event_queue_.pop()();
    }
}

bklib::platform_window::platform_handle window::get_handle() const {
    return {window_.get()};
}

//------------------------------------------------------------------------------
void window::shutdown() {
    push_job_([] {
        ::PostQuitMessage(0);
    });
}

using pw = bklib::platform_window;

void window::listen(pw::on_create callback) { on_create_ = callback; }
void window::listen(pw::on_paint  callback) { on_paint_  = callback; }
void window::listen(pw::on_close  callback) { on_close_  = callback; }
void window::listen(pw::on_resize callback) { on_resize_ = callback; }

using mouse = bklib::mouse;

void window::listen(mouse::on_enter         callback) {}
void window::listen(mouse::on_exit          callback) {}
void window::listen(mouse::on_move          callback) { on_mouse_move_    = callback; }
void window::listen(mouse::on_move_to       callback) { on_mouse_move_to_ = callback; }
void window::listen(mouse::on_mouse_down    callback) { on_mouse_down_    = callback; }
void window::listen(mouse::on_mouse_up      callback) { on_mouse_up_      = callback; }
void window::listen(mouse::on_mouse_wheel_v callback) { on_mouse_wheel_v_ = callback; }
void window::listen(mouse::on_mouse_wheel_h callback) {  }

using kb = bklib::keyboard;

void window::listen(kb::on_keydown callback) { on_keydown_ = callback; }
void window::listen(kb::on_keyup   callback) { on_keyup_   = callback; }

void window::listen(bklib::ime_candidate_list::on_begin  callback) {}
void window::listen(bklib::ime_candidate_list::on_update callback) {}
void window::listen(bklib::ime_candidate_list::on_end    callback) {}

////////////////////////////////////////////////////////////////////////////////
// raw_input
////////////////////////////////////////////////////////////////////////////////
using raw_input = bklib::detail::raw_input;

raw_input::raw_input(LPARAM const lParam)
    : buffer_size_{0}
    , buffer_{}
{
    auto handle = reinterpret_cast<HRAWINPUT>(lParam);

    auto result = ::GetRawInputData(
        handle
      , RID_INPUT
      , 0
      , &buffer_size_
      , sizeof(RAWINPUTHEADER)
    );

    if (result != 0) {
        BK_DEBUG_BREAK(); //TODO
    }

    buffer_ = std::make_unique<char[]>(buffer_size_);

    result = ::GetRawInputData(
        handle
      , RID_INPUT
      , buffer_.get()
      , &buffer_size_
      , sizeof(RAWINPUTHEADER)
    );

    if (result != buffer_size_) {
        BK_DEBUG_BREAK(); //TODO
    }
}

//raw_input::raw_input(const raw_input& other)
//    : buffer_size_{other.buffer_size_}
//    , buffer_{std::make_unique<char[]>(buffer_size_)}
//{
//    std::copy_n(other.buffer_.get(), buffer_size_, buffer_.get());
//}
//
//raw_input& raw_input::operator=(const raw_input& rhs) {
//    buffer_size_ = rhs.buffer_size_;
//
//    auto temp = std::make_unique<char[]>(buffer_size_);
//    std::copy_n(rhs.buffer_.get(), buffer_size_, temp.get());
//        
//    using std::swap;
//    swap(buffer_, temp);
//
//    return *this;
//}

void raw_input::handle_message() {
    auto ptr = reinterpret_cast<RAWINPUT*>(buffer_.get());
    auto const result = ::DefRawInputProc(&ptr, 1, sizeof(RAWINPUTHEADER));
    if (result != S_OK) {
        BK_DEBUG_BREAK(); //TODO
    }
}


//------------------------------------------------------------------------------