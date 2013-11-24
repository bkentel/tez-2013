#pragma once

#include "types.hpp"
#include "callback.hpp"
#include "util.hpp"

namespace bklib {
//==============================================================================
//! Keyboard keys; roughly aligned with ASCII codes.
//==============================================================================
enum class keys : uint8_t {
    NONE
  , SPACE = ' '
  , K0 = '0', K1, K2, K3, K4, K5, K6, K7, K8, K9
  , A  = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
  , NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9
  , NUM_DIV, NUM_MUL, NUM_MIN, NUM_ADD, NUM_DEC, NUM_ENTER, NUM_LCK
  , F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24
  , LEFT, RIGHT, UP, DOWN
  , CTRL_L, CTRL_R
  , ALT_L, ALT_R
  , SHIFT_L, SHIFT_R
  , ENTER
  , INS, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
};

//==============================================================================
//! Key combos.
//==============================================================================
class key_combo {
public:
    using key_t = bklib::keys;
    using pred  = std::greater<>;

    key_combo(key_combo const&) = default;
    key_combo& operator=(key_combo const&) = default;

    key_combo(key_combo&&) = default;
    key_combo& operator=(key_combo&&) = default;

    key_combo() {
        keys_.reserve(4);
    }

    key_combo(std::initializer_list<key_t> const list)
      : keys_{list}
    {
        std::sort(std::begin(keys_), std::end(keys_), pred{});
    }    

    bool add(key_t const k) {
        auto it = std::lower_bound(std::begin(keys_), std::end(keys_), k, pred{});
        if (it != std::end(keys_) && *it == k) {
            return false;
        }

        auto distance = std::distance(std::begin(keys_), it);
        keys_.emplace_back(k);
        std::sort(std::begin(keys_) + distance, std::end(keys_), pred{});

        return true;
    }

    bool add(std::initializer_list<key_t> const list) {
        bool result = false;

        for (auto k : list) {
            result |= add(k);
        }

        return result;
    }

    bool remove(key_t const k) {
        auto it = std::lower_bound(std::begin(keys_), std::end(keys_), k, pred{});
        if (it == std::end(keys_) || *it != k) {
            return false;
        }

        keys_.erase(it);
        return true;
    }

    bool operator==(key_combo const& rhs) const {
        return std::equal(
            std::cbegin(keys_), std::end(keys_), std::cbegin(rhs.keys_)
        );
    }

    bool operator!=(key_combo const& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(key_combo const& rhs) const {
        return std::lexicographical_compare(
            std::cbegin(keys_),     std::end(keys_)
          , std::cbegin(rhs.keys_), std::end(rhs.keys_)
        );
    }

    auto size() const { return keys_.size(); }
    auto empty() const { return keys_.empty(); }

    void clear() {
        keys_.clear();
    }

    auto begin() { return keys_.begin(); }
    auto end() { return keys_.end(); }

    auto begin() const { return keys_.begin(); }
    auto end() const { return keys_.end(); }

    auto cbegin() const { return keys_.cbegin(); }
    auto cend() const { return keys_.cend(); }

    friend std::ostream& operator<<(std::ostream& out, key_combo const& combo);
private:
    std::vector<key_t> keys_;
};

std::ostream& operator<<(std::ostream& out, key_combo const& combo);

//==============================================================================
//! The state of the keyboard.
//==============================================================================
class keyboard {
public:
    using clock      = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using key_t      = bklib::keys;
    using code_t     = std::underlying_type_t<key_t>;

    BK_DECLARE_EVENT(on_keydown,   void (keyboard& state, key_t k));
    BK_DECLARE_EVENT(on_keyup,     void (keyboard& state, key_t k));
    BK_DECLARE_EVENT(on_keyrepeat, void (keyboard& state, key_t k));

    static utf8string const INVALID_KEY_NAME;

    struct record {
        time_point time;    //!<< Time stamp of when the keystate changed.
        bool       is_down; //!<< The key is down.

        explicit operator bool() const BK_NOEXCEPT { return is_down; }
    };

    static utf8string const& key_name(key_t k);
    static key_t key_code(utf8string const& name);
    static key_t key_code(hash hash);

    keyboard();

    record operator[](key_t const k) const {
        auto const i = enum_value(k);
        return state_[i];
    }
    
    bool set_state(key_t k, bool is_down);

    void clear(on_keyup const& f);
private:
    key_combo keys_;
    std::array<record, 0xFF> state_;
};

} //namespace bklib
