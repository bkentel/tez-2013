#pragma once

#include "platform.hpp";
#include "concurrent_queue.hpp"
#include "window.hpp"
#include "ime_windows.hpp"

namespace bklib { namespace win {

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
    using on_create = platform_window::on_create;
    using on_mouse_move_to = platform_window::on_mouse_move_to;

    impl_t_();

    void shutdown();

    HWND handle() const BK_NOEXCEPT {
        return window_.get();
    }

    bool is_running() const BK_NOEXCEPT {
        return running_;
    }

    void do_events();

    void listen(on_create callback) {}
    void listen(on_mouse_move_to callback) {}

    void listen(ime_candidate_list::on_update callback) {
        auto func = [=](ime_candidate_list cl, ime_candidate_list::update_type type) {
            push_event_([=] {
                callback(cl, type);
            });            
        };

        ime_manager_->listen(ime_candidate_list::on_update {func});
    }
private:
    window_handle window_;

    LRESULT local_wnd_proc_(UINT uMsg, WPARAM wParam, LPARAM lParam);   

    static LRESULT CALLBACK wnd_proc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static HWND create_window_(impl_t_* win);

    static void push_job_(invocable job);
    static void push_event_(invocable event);
   
    static void main_();
    static void init_();
    
    static concurrent_queue<invocable> work_queue_;
    static concurrent_queue<invocable> event_queue_;
    static bool running_;
    static DWORD thread_id_;
    static std::unique_ptr<impl::ime_manager> ime_manager_;
};

}} //namespace bklib::win
