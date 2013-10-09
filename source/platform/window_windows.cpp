#include "pch.hpp"

#include "window_windows.hpp"
#include "ime_windows.hpp"
#include "com.hpp"

#pragma comment(lib, "Imm32.lib")

#define BK_WM_ASSOCIATE_TSF (WM_APP + 1)

using window = bklib::platform_window::impl_t_;

//------------------------------------------------------------------------------
bklib::concurrent_queue<window::invocable> window::work_queue_ {};
bklib::concurrent_queue<window::invocable> window::event_queue_ {};
bool                                window::running_ {false};
DWORD                               window::thread_id_ {0};
//std::unique_ptr<bklib::impl::ime_manager> window::ime_manager_;
//------------------------------------------------------------------------------
struct is_not_ok {
    bool operator()(HRESULT const hr) const BK_NOEXCEPT {
        return hr != S_OK;
    }
};

struct api_error : virtual std::exception, virtual boost::exception {};

#define BK_THROW_API_IF(api, value, cond) \
for (auto const pred = cond(value); pred;) { \
    BOOST_THROW_EXCEPTION(api_error {} \
        << boost::errinfo_api_function(#api) \
        << boost::errinfo_errno(value) \
    ); \
} []{}

#define BK_THROW_API(name, value) \
BOOST_THROW_EXCEPTION(api_error {} \
    << boost::errinfo_api_function(#name) \
    << boost::errinfo_errno(value) \
)

namespace {
    //--------------------------------------------------------------------------
    std::once_flag once_flag; //used for initialization
    //--------------------------------------------------------------------------
    template <typename T>
    T* get_user_data(HWND hWnd) {
        ::SetLastError(0);
        auto const result = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    
        if (result == 0) {
            BK_THROW_API_IF(GetWindowLongPtrW,
                ::GetLastError(), std::bind1st(std::not_equal_to<DWORD>{}, 0));
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
            BK_THROW_API_IF(SetWindowLongPtrW,
                ::GetLastError(), std::bind1st(std::not_equal_to<DWORD>{}, 0));
        }

        return result;
    }
} //namespace
//------------------------------------------------------------------------------
void window::push_job_(invocable job) {
    work_queue_.push(std::move(job));

    auto const result = ::PostThreadMessageW(thread_id_, WM_NULL, 0, 0);
    if (result == 0) {
        BK_THROW_API(PostThreadMessageW, ::GetLastError());
    }
}
//------------------------------------------------------------------------------
void window::push_event_(invocable event) {
    event_queue_.push(std::move(event));
}
//------------------------------------------------------------------------------
void window::init_() {
    std::call_once(once_flag, [] {
        ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

        BK_THROW_ON_COM_ERROR(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
        BK_THROW_API_EQUAL(::ImmDisableIME(static_cast<DWORD>(-1)), FALSE);

        std::thread window_thread(window::main_);
        thread_id_ = ::GetThreadId(window_thread.native_handle());
        window_thread.detach();

        while (!running_ || thread_id_ == 0) {}
    });
}
//------------------------------------------------------------------------------
void window::main_() {
    MSG msg {0};
    BK_THROW_ON_COM_ERROR(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

    //enusure the thread has a message queue before continuing
    ::PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    //ime_manager_ = std::make_unique<impl::ime_manager>(); //TODO

    running_ = true;
    BK_SCOPE_EXIT({running_ = false;});

    for (;;) {
        auto const result = ::GetMessageW(&msg, 0, 0, 0);

        while (!work_queue_.is_empty()) {
            work_queue_.pop()();
        }

        switch (result) {
        //OK
        case 1:
            if (msg.message == BK_WM_ASSOCIATE_TSF) {
                //ime_manager_->associate(msg.hwnd); //TODO
            }

            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);

            break;
        //WM_QUIT
        case 0:
            return;
        //Error
        default:
            return;
        }
    }
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
            BK_THROW_API(CreateWindowExW, ::GetLastError());
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
    BK_DEBUG_BREAK();
    ::PostQuitMessage(-1);
    return 0;
} catch (...) {
    BK_DEBUG_BREAK();
    ::PostQuitMessage(-1);
    return 0;
}
//------------------------------------------------------------------------------
LRESULT window::local_wnd_proc_(UINT const uMsg, WPARAM const wParam, LPARAM const lParam) {
    switch (uMsg) {
    case WM_DESTROY :
        ::PostQuitMessage(0);
        break;
    case WM_MOUSEMOVE :
        push_event_([=] {
            auto const x = static_cast<int>(lParam & 0xFFFF);
            auto const y = static_cast<int>(lParam >> 16);
            
            //if (pw_.on_mouse_move_to_) pw_.on_mouse_move_to_(x, y);
            //event_mouse_move(x, y);
        });
        break;
    case WM_SIZE:
        push_event_([=] {
            RECT r;
            ::GetClientRect(handle(), &r);
            
            //event_size(r.right - r.left, r.bottom - r.top);
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
//------------------------------------------------------------------------------
void window::shutdown() {
    push_job_([] {
        ::PostQuitMessage(0);
    });
}



void window::listen(bklib::platform_window::on_create callback) {}
void window::listen(bklib::platform_window::on_close  callback) {}
void window::listen(bklib::platform_window::on_resize callback) {}

void window::listen(bklib::mouse::on_enter   callback) {}
void window::listen(bklib::mouse::on_exit    callback) {}
void window::listen(bklib::mouse::on_move    callback) {}
void window::listen(bklib::mouse::on_move_to callback) {}

void window::listen(bklib::ime_candidate_list::on_begin  callback) {}
void window::listen(bklib::ime_candidate_list::on_update callback) {}
void window::listen(bklib::ime_candidate_list::on_end    callback) {}
