#pragma once

#include <type_traits>
#include <ostream>
#include <jsoncpp/json.h>

#include "types.hpp"
#include "exception.hpp"
#include "json_forward.hpp"

namespace bklib { namespace json {
//==============================================================================
namespace error {
    struct base      : virtual bklib::exception_base {};
    struct bad_type  : virtual base {};
    struct bad_size  : virtual base {};
    struct bad_index : virtual base {};

    BK_DEFINE_EXCEPTION_INFO(info_expected_type, utf8string);
    BK_DEFINE_EXCEPTION_INFO(info_actual_type,   utf8string);
    BK_DEFINE_EXCEPTION_INFO(info_expected_size, size_t);
    BK_DEFINE_EXCEPTION_INFO(info_actual_size,   size_t);
    BK_DEFINE_EXCEPTION_INFO(info_index,         index);
    BK_DEFINE_EXCEPTION_INFO(info_location,      utf8string);

    std::ostream& operator<<(std::ostream& out, base const& e);
} //namespace error
//==============================================================================
//! @throws json::error::bad_type if !json.isArray().
//! @return json
//==============================================================================
cref require_array(cref json);
//==============================================================================
//! @throws json::error::bad_type if !json.isObject().
//! @return json
//==============================================================================
cref require_object(cref json);
//==============================================================================
//! @pre json.isObject().
//! @throws json::error::bad_index if @c index is invalid.
//! @return A @c cref to the value at @c index.
//==============================================================================
cref require_key(cref json, utf8string const& index);
//==============================================================================
//! @pre json.isArray().
//! @throws json::error::bad_index if @c index is invalid.
//! @return A @c cref to the value at @c index.
//==============================================================================
cref require_key(cref json, size_t index);
//==============================================================================
//! @throws json::error::bad_type if @c !json.isString().
//! @return @c json as a string.
//==============================================================================
utf8string require_string(cref json);
//==============================================================================
namespace detail {
    void for_each_element_skip_on_fail_on_fail_(
        error::base const& e
      , size_t      const  index
    );
} //namespace detail
//==============================================================================
//! Iterate through each array element in @c json and apply the unary function
//! @action to each element. If @c action fails due to a json related error, the
//! error is logged and iteration continues at the next element.
//!
//! @throws json::error::bad_type if !json.isArray().
//==============================================================================
template <typename F>
void for_each_element_skip_on_fail(cref json, F&& action) {
    json::require_array(json);

    auto const size = json.size();
    for (size_t i = 0; i < size; ++i) {
        try {
            action(require_key(json, i));
        } catch (error::base const& e) {
            detail::for_each_element_skip_on_fail_on_fail_(e, i);
        }
    }
}
//==============================================================================


////==============================================================================
////!
////==============================================================================
//class input_stack {
//public:
//    //--------------------------------------------------------------------------
//    using pointer_t = Json::Value const*;
//    using record_t  = std::pair<index_t, pointer_t>;
//    //--------------------------------------------------------------------------
//    explicit input_stack(std::istream& in);
//
//    explicit input_stack(std::istream&& in)
//      : input_stack(in)
//    {
//    }
//
//    explicit input_stack(utf8string const& filename)
//      : input_stack(std::ifstream{filename})
//    {
//    }
//    //--------------------------------------------------------------------------
//    input_stack& step_into(index_t const& index);
//    //--------------------------------------------------------------------------
//    input_stack& step_out();
//    //--------------------------------------------------------------------------
//    utf8string require_string(index_t const& index) const;
//    //--------------------------------------------------------------------------
//    size_t size() const {
//        return current_->size();
//    }
//    //--------------------------------------------------------------------------
//    friend std::ostream& operator<<(std::ostream& lhs, input_stack const& rhs);
//private:
//    pointer_t get_index_(index_t const& index) const;
//
//    Json::Value           root_;
//    pointer_t             current_;
//    std::vector<record_t> stack_; 
//};
//
//std::ostream& operator<<(std::ostream& lhs, input_stack const& rhs);

} //namespace json
} //namespace bklib
