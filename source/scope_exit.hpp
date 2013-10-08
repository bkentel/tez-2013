#pragma once

#include "config.hpp"

namespace bklib {

template <typename F>
struct scope_exit {
    scope_exit(F f) : f(f) {}
    ~scope_exit() { f(); }
    F f;
};

template <typename F>
scope_exit<F> make_scope_exit(F f) {
    return scope_exit<F>(f);
};

} //bklib

#define BK_SCOPE_EXIT(CODE) \
auto BK_UNIQUE_ID(scope_exit_) = ::bklib::make_scope_exit([&]CODE)
