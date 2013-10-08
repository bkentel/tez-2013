#pragma once

#include <memory>

#include "ime.hpp"
#include "callback.hpp"

namespace bklib {

class platform_window {
public:
    class impl_t_;
    friend impl_t_;
    
    ~platform_window();

    platform_window(platform_string title, unsigned width, unsigned height);

    using on_create        = callback<struct tag_on_create, void()>;
    using on_close         = callback<struct tag_on_close, void()>;
    using on_mouse_move_to = callback<struct tag_on_mouse_move_to, void(int, int)>;

    using on_ime_composition_begin  = callback<struct tag_on_ime_composition_begin, void(ime_composition&)>;
    using on_ime_composition_update = callback<struct tag_on_ime_composition_update, void(ime_composition&)>;
    using on_ime_composition_end    = callback<struct tag_on_ime_composition_end, void(ime_composition&)>;

    void listen(on_create callback);
    void listen(on_mouse_move_to callback);
    
    void listen(ime_candidate_list::on_begin callback);
    void listen(ime_candidate_list::on_update callback);
    void listen(ime_candidate_list::on_end callback);

    void do_events();
private:
    std::unique_ptr<impl_t_> impl_;
};

} //namespace bklib
