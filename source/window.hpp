#pragma once

#include <memory>

#include "types.hpp"
#include "ime.hpp"
#include "callback.hpp"

namespace bklib {

class mouse {
public:
    BK_DECLARE_EVENT(on_enter, void(int x, int y));
    BK_DECLARE_EVENT(on_exit,  void(int x, int y));
    
    BK_DECLARE_EVENT(on_move,    void(int dx, int dy));
    BK_DECLARE_EVENT(on_move_to, void(int dx, int dy));
};

class platform_window {
public:
    class impl_t_;
    friend impl_t_;
    
    ~platform_window();
    explicit platform_window(platform_string title);

    BK_DECLARE_EVENT(on_create, void());
    BK_DECLARE_EVENT(on_close,  void());
    BK_DECLARE_EVENT(on_resize, void(unsigned w, unsigned h));;

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

    void do_events();
private:
    std::unique_ptr<impl_t_> impl_;
};

} //namespace bklib
