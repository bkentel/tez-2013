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
//std::unique_ptr<bklib::impl::ime_manager> window::ime_manager_;
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
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct raw_input {
    explicit raw_input(LPARAM lParam) {
        auto handle = reinterpret_cast<HRAWINPUT>(lParam);

        UINT size {0};
        auto result = ::GetRawInputData(
            handle
          , RID_INPUT
          , 0
          , &size
          , sizeof(RAWINPUTHEADER)
        );

        if (result != 0) {
            BK_DEBUG_BREAK(); //TODO
        }

        buffer_ = std::make_unique<char[]>(size);

        result = ::GetRawInputData(
            handle
          , RID_INPUT
          , buffer_.get()
          , &size
          , sizeof(RAWINPUTHEADER)
        );

        if (result != size) {
            BK_DEBUG_BREAK(); //TODO
        }
    }

    RAWINPUT const* operator->() const {
        return reinterpret_cast<RAWINPUT*>(buffer_.get());
    }

    bool is_mouse() const { return (*this)->header.dwType == RIM_TYPEMOUSE; }
    
    RAWMOUSE const& mouse() const {
        BK_ASSERT(is_mouse());
        return (*this)->data.mouse;
    }

    void handle() {
        auto i = reinterpret_cast<RAWINPUT*>(buffer_.get());
        auto const result = ::DefRawInputProc(&i, 1, sizeof(RAWINPUTHEADER));
        if (result != S_OK) {
            BK_DEBUG_BREAK(); //TODO
        }
    }

    std::unique_ptr<char[]> buffer_;
};

//------------------------------------------------------------------------------
LRESULT window::local_wnd_proc_(
    UINT   const uMsg
  , WPARAM const wParam
  , LPARAM const lParam
) {
    switch (uMsg) {
    case WM_INPUT : {
        auto input = raw_input{lParam};
        if (input.is_mouse()) {
            auto& mouse = input.mouse();
            auto const x = mouse.lLastX;
            auto const y = mouse.lLastY;

            push_event_([=] {
                if (on_mouse_move_) on_mouse_move_(x, y);
            });

            return 0;
        } else {
            input.handle();
        }
    } break;
    case WM_PAINT :
        push_event_([=] {
            if (on_paint_) on_paint_();
        });
        break;
    case WM_ERASEBKGND :
        return 0;
    case WM_DESTROY :
        ::PostQuitMessage(0);
        break;
    case WM_MOUSEMOVE :
        push_event_([=] {
            auto const x = static_cast<int>(lParam & 0xFFFF);
            auto const y = static_cast<int>(lParam >> 16);

            if (on_mouse_move_to_) on_mouse_move_to_(x, y);
        });
        break;
    case WM_SIZE:
        push_event_([=] {
            RECT r {};
            ::GetClientRect(handle(), &r);
            
            if (on_resize_) on_resize_(
                r.right  - r.left
              , r.bottom - r.top
            );
        });
        break;
    default :
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

void window::listen(mouse::on_enter   callback) {}
void window::listen(mouse::on_exit    callback) {}
void window::listen(mouse::on_move    callback) { on_mouse_move_ = callback; }
void window::listen(mouse::on_move_to callback) { on_mouse_move_to_ = callback; }

void window::listen(bklib::ime_candidate_list::on_begin  callback) {}
void window::listen(bklib::ime_candidate_list::on_update callback) {}
void window::listen(bklib::ime_candidate_list::on_end    callback) {}
