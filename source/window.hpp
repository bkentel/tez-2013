#pragma once

#include <memory>

//#define BOOST_CB_DISABLE_DEBUG
#include <boost/circular_buffer.hpp>

#include "types.hpp"
#include "ime.hpp"
#include "callback.hpp"

namespace bklib {

template <typename T, size_t N = sizeof(T)>
struct print_size_of;

template <typename EnumType>
class bit_flags {
public:
    static_assert(std::is_enum<EnumType>::value, "must be an enum type.");
    using storage_type = std::underlying_type_t<EnumType>;

    bit_flags() BK_NOEXCEPT
        : value_{0}
    {
    }

    bit_flags(EnumType const value) BK_NOEXCEPT
        : value_{ static_cast<storage_type>(value) }
    {
    }

    inline bool operator&(EnumType const flag) const BK_NOEXCEPT {
        return (value_ & static_cast<storage_type>(flag)) != 0;
    }

    inline bit_flags operator|(EnumType const flag) const BK_NOEXCEPT {
        return { value_ | static_cast<storage_type>(flag) };
    }

    inline bit_flags operator|=(EnumType const flag) BK_NOEXCEPT {
        return (*this = *this | flag);
    }

    void reset() BK_NOEXCEPT { value_ = 0; }
    void reset(EnumType const value) BK_NOEXCEPT { value_ = static_cast<storage_type>(value); }
private:
    bit_flags(storage_type value) BK_NOEXCEPT : value_{value} {}
    storage_type value_;
};

class mouse {
public:
    using clock = std::chrono::high_resolution_clock;

    BK_DECLARE_EVENT(on_enter, void (mouse& m));
    BK_DECLARE_EVENT(on_exit,  void (mouse& m));

    BK_DECLARE_EVENT(on_move,    void (mouse& m, int x, int y));
    BK_DECLARE_EVENT(on_move_to, void (mouse& m, int dx, int dy));

    BK_DECLARE_EVENT(on_mouse_down,    void (mouse& m, unsigned button));
    BK_DECLARE_EVENT(on_mouse_up,    void (mouse& m, unsigned button));

    BK_DECLARE_EVENT(on_mouse_wheel_v,    void (mouse& m, int delta));
    BK_DECLARE_EVENT(on_mouse_wheel_h,    void (mouse& m));

    enum class update_type : char {
        none              = 0
      , absolute_position = 1 << 0
      , relative_position = 1 << 1
      , button            = 1 << 2
      , wheel_vertical    = 1 << 3
      , wheel_horizontal  = 1 << 4
    };

    enum class button_state : char {
        unknown, is_up, went_up, is_down, went_down
    };

    struct record {
        clock::time_point           time;
        std::int32_t                x;
        std::int32_t                y;
        std::array<button_state, 5> buttons;
        bit_flags<update_type>      flags;
        std::int16_t                wheel_delta;
    };

    record history(size_t n = 0) const {
        auto const size = history_.size();
        BK_ASSERT(n < size);

        auto it = history_.begin() + n;
        record result = *it;
        return result;
    }

    void push(record rec) {
        history_.push_front(rec);
    }

    mouse() {
        static auto const state = button_state::unknown;

        record const rec {
            clock::now()
          , 0, 0
          , {{state, state, state, state, state}}
          , update_type::none
          , 0
        };

        history_.set_capacity(100);
        push(rec);
    }
private:
    boost::circular_buffer<record> history_;
    clock::time_point buttons_[5];
};

class platform_window {
public:
    struct platform_handle;

    class impl_t_;
    friend impl_t_;

    enum class state {
        starting, running, finished_error, finished_ok
    };

    ~platform_window();
    explicit platform_window(platform_string title);

    BK_DECLARE_EVENT(on_create, void());
    BK_DECLARE_EVENT(on_paint,  void());
    BK_DECLARE_EVENT(on_close,  void());
    BK_DECLARE_EVENT(on_resize, void(unsigned w, unsigned h));

    void listen(on_create callback);
    void listen(on_paint  callback);
    void listen(on_close  callback);
    void listen(on_resize callback);

    void listen(mouse::on_enter   callback);
    void listen(mouse::on_exit    callback);
    void listen(mouse::on_move    callback);
    void listen(mouse::on_move_to callback);

    void listen(ime_candidate_list::on_begin  callback);
    void listen(ime_candidate_list::on_update callback);
    void listen(ime_candidate_list::on_end    callback);

    bool is_running() const;

    std::future<int> result_value();

    void do_events();

    platform_handle get_handle() const;
private:
    std::unique_ptr<impl_t_> impl_;
};

#if defined(BOOST_OS_WINDOWS)
struct platform_window::platform_handle {
    operator HWND() const { return value; }
    HWND value;
};
#endif

} //namespace bklib
