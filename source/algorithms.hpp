#pragma once

namespace bklib {

//==============================================================================
//! For every element in the container @c matching the predicate @c test,
//! acculate via @sum.
//!
//! @tparam Container Forward iteratable container.
//! @tparam Value Value type for the initial value for the sum.
//! @tparam Test Unary predicate(Container::value_type).
//! @tparam Sum Binary function(Value, Container::value_type)
//==============================================================================
template <typename Container, typename Value, typename Test, typename Sum>
Value accumulate_if(Container const& c, Value init, Test test, Sum sum) {
    auto it  = std::cbegin(c);
    auto end = std::cend(c);

    Value value = init;

    while (end != (it = std::find_if(it, end, test))) {
        value = sum(value, *it++);
    }

    return value;
}

} //namespace bklib
