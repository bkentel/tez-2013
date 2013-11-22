#include "pch.hpp"
#include "keyboard.hpp"

using namespace bklib;
using kb = bklib::keyboard;

////////////////////////////////////////////////////////////////////////////////
// bklib::key_combo
////////////////////////////////////////////////////////////////////////////////
std::ostream& bklib::operator<<(std::ostream& out, key_combo const& combo) {
    out << "[";
    for (auto k : combo.keys_) {
        out << keyboard::key_name(k) << ", ";
    }
    out << "]";

    return out;
}

////////////////////////////////////////////////////////////////////////////////
// bklib::keyboard
////////////////////////////////////////////////////////////////////////////////
namespace {
    static boost::container::flat_map<kb::key_t, utf8string> key_string_map;
    static boost::container::flat_map<hash, kb::key_t>       hash_key_map;

    static std::once_flag maps_flag;

    #define BK_ADD_KEY_HASH(ENUM)\
    [&] {\
        auto mapping = ::std::make_pair(\
              ::bklib::utf8string_hash(#ENUM), ENUM\
        );\
    \
        auto const result = hash_key_map.emplace(std::move(mapping));\
        BK_ASSERT(result.second && "collision");\
    }(); \
    [&] {\
        auto mapping = ::std::make_pair(\
              ENUM, #ENUM\
        );\
    \
        auto const result = key_string_map.emplace(std::move(mapping));\
        BK_ASSERT(result.second && "duplicate");\
    }()

    void init_maps() {
        using KEY = ::bklib::keys;
        BK_ADD_KEY_HASH(KEY::NUM_0); BK_ADD_KEY_HASH(KEY::NUM_1);
        BK_ADD_KEY_HASH(KEY::NUM_2); BK_ADD_KEY_HASH(KEY::NUM_3);
        BK_ADD_KEY_HASH(KEY::NUM_4); BK_ADD_KEY_HASH(KEY::NUM_5);
        BK_ADD_KEY_HASH(KEY::NUM_6); BK_ADD_KEY_HASH(KEY::NUM_7);
        BK_ADD_KEY_HASH(KEY::NUM_8); BK_ADD_KEY_HASH(KEY::NUM_9);

        BK_ADD_KEY_HASH(KEY::LEFT); BK_ADD_KEY_HASH(KEY::RIGHT);
        BK_ADD_KEY_HASH(KEY::UP);   BK_ADD_KEY_HASH(KEY::DOWN);

        BK_ADD_KEY_HASH(KEY::SPACE);
    }

    #undef BK_ADD_KEY_HASH
} //namespace

utf8string const& kb::key_name(key_t k) {
    static utf8string const NOT_FOUND = {"UNKNOWN_KEY"};

    std::call_once(maps_flag, init_maps);

    auto it = key_string_map.find(k);
    return it != std::cend(key_string_map) ? it->second : NOT_FOUND;
}

bklib::keys kb::key_code(utf8string const& name) {
    return key_code(bklib::utf8string_hash(name));
}

bklib::keys kb::key_code(hash const hash) {
    std::call_once(maps_flag, init_maps);

    auto it = hash_key_map.find(hash);
    return it != std::cend(hash_key_map) ? it->second : keys::NONE;
}


kb::keyboard() {
    record const r = {clock::now(), false};
    std::fill(std::begin(state_), std::end(state_), r);
}
    
bool kb::set_state(key_t const k, bool const is_down) {
    auto const i = enum_value(k);

    if (state_[i].is_down == is_down) return true;

    state_[i].is_down = is_down;
    state_[i].time    = clock::now();

    if (is_down) {
        keys_.add(k);
    } else {
        keys_.remove(k);
    }

    //std::cout << keys_ << std::endl;

    return false;
}

void kb::clear(on_keyup const& f) {
    keys_.clear();

    auto const now = clock::now();

    code_t code = 0;
    for (auto& r : state_) {
        if (r.is_down) {
            if (f) f(*this, static_cast<key_t>(code));
            r.is_down = false;
            r.time    = now;
        }
        code++;
    }
}

