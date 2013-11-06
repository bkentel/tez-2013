#pragma once

#include <chrono>
#include <vector>
#include <functional>

#include "config.hpp"

namespace bklib {

class timekeeper {
public:
    using clock      = std::chrono::high_resolution_clock;
    using duration   = clock::duration;
    using time_point = clock::time_point;
    using delta      = std::chrono::milliseconds;
    using callback   = std::function<void (delta dt)>;
    
    struct handle { size_t id; };

    struct record {
        timekeeper::handle     handle;
        timekeeper::callback   callback;
        timekeeper::duration   period;
        timekeeper::time_point deadline;
    };

    timekeeper();

    //! Register a callback @c f to be called every @c period of time.
    template <typename T>
    handle register_event(T period, callback f) {
        return register_event_(
            std::chrono::duration_cast<duration>(period)
          , std::move(f)
        );
    }

    //! Update the time and execute all callbacks which have met or exceeded
    //! their deadlines.
    void update();
private:
    handle register_event_(duration period, callback f);

    std::vector<record>  records_;
    std::vector<record*> heap_;
};

} // namespace bklib
