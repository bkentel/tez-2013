#include "pch.hpp"
#include "timekeeper.hpp"

using tk = bklib::timekeeper;

namespace {
    bool predicate(tk::record const* a, tk::record const* b) BK_NOEXCEPT {
        return a->deadline > b->deadline;
    }
}

//==============================================================================
//! 
//==============================================================================
tk::timekeeper() {
    records_.reserve(20);
}
//==============================================================================
//! 
//==============================================================================
tk::handle tk::register_event_(duration period, callback f) {
    BK_ASSERT(f);
    BK_ASSERT(period.count() > 0);

    auto const now = clock::now();
    auto const deadline = now + period;

    timekeeper::handle const handle = { records_.size() };

    records_.emplace_back(record {
        handle, std::move(f), period, deadline            
    });

    bool const make_heap = records_.capacity() == records_.size();

    if (!make_heap) {
        heap_.emplace_back(std::addressof(records_.back()));
        std::push_heap(std::begin(heap_), std::end(heap_), predicate);
    } else {
        heap_.clear();
        std::transform(
            std::begin(records_), std::end(records_)
            , std::back_inserter(heap_)
            , [](record& r) -> record* { return &r; }
        );
        std::make_heap(std::begin(heap_), std::end(heap_), predicate);
    }

    return handle;
}
//==============================================================================
//! 
//==============================================================================
void tk::update() {
    auto const now = clock::now();
    duration dt = std::chrono::seconds(1);

    while (!heap_.empty() && dt.count() >= 0) {
        std::pop_heap(std::begin(heap_), std::end(heap_), predicate);
        auto rec = heap_.back();

        dt = now - rec->deadline;
        if (dt.count() >= 0) {
            rec->callback(std::chrono::duration_cast<delta>(dt + rec->period));
            rec->deadline = now + rec->period;
        }

        std::push_heap(std::begin(heap_), std::end(heap_), predicate);
    }
}
