#pragma once

#include "platform.hpp"
#include "com.hpp"
#include "concurrent_queue.hpp"
#include "window.hpp"
#include "ime_windows.hpp"
#include "exception.hpp"

namespace bklib {

struct hwnd_deleter {
    typedef HWND pointer;
    void operator()(pointer p) const BK_NOEXCEPT {
        ::DestroyWindow(p);
    }
};

using window_handle = std::unique_ptr<HWND, hwnd_deleter>;

//!=============================================================================
//! 
//!=============================================================================
class platform_window::impl_t_ {
    impl_t_(impl_t_ const&) = delete;
    impl_t_& operator=(impl_t_ const&) = delete;
public:
    using invocable        = std::function<void()>;

    impl_t_();

    void shutdown();

    HWND handle() const BK_NOEXCEPT { return window_.get(); }

    bool is_running() const BK_NOEXCEPT { return state_ == state::running; }

    std::future<int> result_value() {
        return result_.get_future();
    }

    void do_events();

    void listen(on_create callback);
    void listen(on_close  callback);
    void listen(on_resize callback);

    void listen(mouse::on_enter   callback);
    void listen(mouse::on_exit    callback);
    void listen(mouse::on_move    callback);
    void listen(mouse::on_move_to callback);

    void listen(ime_candidate_list::on_begin  callback);
    void listen(ime_candidate_list::on_update callback);
    void listen(ime_candidate_list::on_end    callback);
private:
    window_handle window_;

    on_create on_create_;
    on_close  on_close_;
    on_resize on_resize_;

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
    //static bool running_;
    static DWORD thread_id_;
    static state state_;
    static std::promise<int> result_;
    //static std::unique_ptr<impl::ime_manager> ime_manager_;
};

} //namespace bklib