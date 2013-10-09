#pragma once

#include <memory>
#include <type_traits>

#include "platform.hpp"

namespace bklib { namespace win {
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
struct bstr_deleter {
    typedef BSTR pointer;
    void operator()(pointer s) const {
        ::SysFreeString(s);
    }
};

using bstr = std::unique_ptr<BSTR, bstr_deleter>;
//------------------------------------------------------------------------------
template <typename T>
struct com_deleter {
    static_assert(std::is_base_of<::IUnknown, T>::value,
        "Can only be used with COM types."
    );

    typedef T* pointer;

    void operator()(pointer p) const BK_NOEXCEPT {
        if (p != nullptr) {
            p->Release();
        }
    }
};

template <typename T>
using com_ptr = std::unique_ptr<T, com_deleter<T>>;
//------------------------------------------------------------------------------
template <typename T>
inline com_ptr<T> query_interface(IUnknown& unk) {
    T* result = nullptr;
    HRESULT const hr = unk.QueryInterface(&result);
    if (FAILED(hr)) {
        BK_DEBUG_BREAK();
        throw "TODO";
    }

    return com_ptr<T> {result};
}

template <typename T, typename U>
inline com_ptr<T> query_interface(com_ptr<U>& cp) {
    return query_interface<T>(*cp);
}
//------------------------------------------------------------------------------
template <typename R, typename F>
auto make_com_ptr_from(F function) -> com_ptr<R> {
    R* result = nullptr;
    HRESULT const hr = function(&result);
    if (FAILED(hr)) {
        BK_DEBUG_BREAK();
        throw "TODO";
    }
    return com_ptr<R> {result};
}
//------------------------------------------------------------------------------
}} //namespace bklib::win
