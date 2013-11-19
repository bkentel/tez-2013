#pragma once

#include <functional>
#include <type_traits>

namespace bklib {

template <typename R, typename... Args>
struct function_return_type;

template <typename R, typename... Args>
struct function_return_type<R (Args...)> {
    using type = R;
};

template <typename T>
using function_return_type_t = typename function_return_type<T>::type;

template <typename Tag, typename Signature>
struct callback {
    static_assert(
        std::is_void<function_return_type_t<Signature>>::value
      , "callbacks must return void"
    );

    callback() = default;
    callback(callback const&) = default;
    callback(callback&&) = default;
    callback& operator=(callback const&) = default;
    callback& operator=(callback&&) = default;

    template <typename F>
    callback(F&& function) : value{std::forward<F>(function)} {}

    template <typename F>
    callback& operator=(F&& function) {
        value = std::forward<F>(function);
        return *this;
    }

    //--------------------------------------------------------------------------
    void swap(callback& other) BK_NOEXCEPT {
        using std::swap;
        swap(value, other.value);
    }
    //--------------------------------------------------------------------------

    template <typename... Args>
    void operator()(Args&&... args) const {
        value(std::forward<Args>(args)...);
    }

    explicit operator bool() const BK_NOEXCEPT {
        return !!value;
    }

    std::function<Signature> value;
};

} //namespace bklib

#define BK_DECLARE_EVENT(name, sig) using name = ::bklib::callback<struct tag_##name, sig>
