#include "pch.hpp"
#include "ime.hpp"

#if defined(BK_PLATFORM_WINDOWS)
#   include "platform/ime_windows.hpp"
#endif

using cl = ::bklib::ime_candidate_list;

cl::~ime_candidate_list() {
}

cl::ime_candidate_list(bklib::non_owning_ptr<impl_t_> impl)
    : impl_ {impl}
{
    BK_ASSERT(impl != nullptr);
}

//cl::strings_t::const_iterator cl::cbegin() const {
//    return impl_->cbegin();
//}
//
//cl::strings_t::const_iterator cl::cend() const {
//    return impl_->cend();
//}
//
//cl::strings_t::const_iterator cl::page_begin() const {
//    return impl_->page_begin();
//}
//
//cl::strings_t::const_iterator cl::page_end() const {
//    return impl_->page_end();
//}
//
//size_t cl::size() const {
//    return impl_->size();
//}
//
//size_t cl::selection() const {
//    return impl_->selection();
//}
//
//size_t cl::page_selection() const {
//    return impl_->page_selection();
//}
//
//void cl::lock() {
//    impl_->lock();
//}
//
//void cl::unlock() {
//    impl_->unlock();
//}
