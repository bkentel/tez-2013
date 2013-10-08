#pragma once

#include <functional>

namespace bklib {

template <typename Id, typename Sig>
struct callback {
    callback() = default;

    template <typename F>
    callback(F function) : value {function} {}

    template <typename F>
    callback& operator=(F function) {
        value = std::move(function);
        return *this;
    }

    callback& operator=(callback&& other) {
        using std::swap;

        swap(value, other.value);

        return *this;
    }

    template <typename... Args>
    void operator()(Args&&... args) {
        value(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(Args&&... args) const {
        value(std::forward<Args>(args)...);
    }

    explicit operator bool() const BK_NOEXCEPT {
        return !!value;
    }

    std::function<Sig> value;
};

} //namespace bklib

#define BK_DECLARE_EVENT(name, sig) using name = ::bklib::callback<struct tag_##name, sig>
