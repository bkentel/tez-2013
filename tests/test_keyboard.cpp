#include "pch.hpp"

#include <gtest/gtest.h>
#include "keyboard.hpp"

using namespace bklib;

//------------------------------------------------------------------------------
TEST(Keyboard, MappingsValid) {
    utf8string const test_name = {"KEY::UP"};
    keys       const test_key  = keys::UP;

    auto const name = keyboard::key_name(test_key);
    ASSERT_EQ(test_name, name);

    auto const key = keyboard::key_code(name);
    ASSERT_EQ(test_key, key);
}
//------------------------------------------------------------------------------
TEST(Keyboard, MappingsInvalid) {
    auto const result_name = keyboard::key_name(static_cast<keys>(0xFF));
    ASSERT_EQ(result_name, keyboard::INVALID_KEY_NAME);

    auto const result_key = keyboard::key_code("blah");
    ASSERT_EQ(result_key, keys::NONE);
}
//------------------------------------------------------------------------------
namespace {
    void check_key(keyboard& kb, keys const key) {
        ASSERT_FALSE(kb[key].is_down);

        ASSERT_FALSE(kb.set_state(key, false));
        ASSERT_FALSE(kb.set_state(key, true));
        ASSERT_TRUE(kb.set_state(key, true));

        kb.clear([key](keyboard& kb, keys const clear_key) {
            ASSERT_EQ(clear_key, key);
            ASSERT_FALSE(kb[clear_key].is_down);
        });
    }
}

TEST(Keyboard, SetGet) {
    keyboard kb;

    for (uint8_t i = 0; i < 0xFF; ++i) {
        auto const key = static_cast<keys>(i);
        check_key(kb, key);
    }
}
//------------------------------------------------------------------------------
TEST(KeyCombo, DefaultConstruct) {
    key_combo const combo;
    
    ASSERT_TRUE(combo.empty());
    ASSERT_EQ(combo.size(), 0);
}
//------------------------------------------------------------------------------
TEST(KeyCombo, Construct) {
    key_combo const combo = {keys::SHIFT_R, keys::SPACE};
    
    ASSERT_FALSE(combo.empty());
    ASSERT_EQ(combo.size(), 2);
}
//------------------------------------------------------------------------------
TEST(KeyCombo, AddOne) {
    key_combo combo;
    
    ASSERT_TRUE(combo.add(keys::ALT_R));
    ASSERT_FALSE(combo.add(keys::ALT_R));

    ASSERT_FALSE(combo.empty());
    ASSERT_EQ(combo.size(), 1);
}
//------------------------------------------------------------------------------
TEST(KeyCombo, AddMulti) {
    key_combo combo;
    
    ASSERT_TRUE(combo.add({keys::ALT_R, keys::D}));
    ASSERT_FALSE(combo.empty());
    ASSERT_EQ(combo.size(), 2);

    ASSERT_FALSE(combo.add({keys::ALT_R, keys::D}));
    ASSERT_TRUE(combo.add({keys::CTRL_R, keys::D}));
    ASSERT_TRUE(combo.add({keys::ALT_R, keys::E}));

    ASSERT_EQ(combo.size(), 4);
}
//------------------------------------------------------------------------------
TEST(KeyCombo, Remove) {
    key_combo combo = {keys::CTRL_L, keys::SPACE};
    
    ASSERT_FALSE(combo.remove(keys::CTRL_R));
    ASSERT_TRUE(combo.remove(keys::CTRL_L));
    ASSERT_TRUE(combo.remove(keys::SPACE));

    ASSERT_EQ(combo.size(), 0);
    ASSERT_TRUE(combo.empty());
}
//------------------------------------------------------------------------------
TEST(KeyCombo, ComboComparison) {
    key_combo const combo_a = {keys::CTRL_L, keys::SPACE};
    key_combo const combo_b = {keys::CTRL_R, keys::SPACE};

    ASSERT_EQ(combo_a, combo_a);
    ASSERT_EQ(combo_b, combo_b);

    ASSERT_NE(combo_a, combo_b);
    ASSERT_NE(combo_b, combo_a);

    ASSERT_FALSE(combo_a < combo_a);
    ASSERT_FALSE(combo_b < combo_b);
}
//------------------------------------------------------------------------------
//////////////////////////
#include "game/hotkeys.hpp"
#include "json.hpp"

using namespace tez;
//------------------------------------------------------------------------------
TEST(HotkeysParser, FromFile) {
    auto bindings = tez::bindings_parser("./data/bindings.def");
    tez::bindings_parser::map_t map = std::move(bindings);

    bklib::key_combo const combo {bklib::keys::LEFT};
    auto it = map.find(combo);

    ASSERT_NE(it, map.cend());
    ASSERT_EQ(it->second, tez::game_command::DIR_WEST);
}
//------------------------------------------------------------------------------
#define BK_ASSERT_THROWS_AND(expr, exception, check)\
for (bool BK_ASSERT_THROWS_AND_passed = false; !BK_ASSERT_THROWS_AND_passed;) {\
    try {\
        expr;\
    } catch (exception const& e) {\
        check;\
    } catch (...) {\
    }\
\
    ASSERT_TRUE(BK_ASSERT_THROWS_AND_passed);\
} []{}()

TEST(HotkeysParser, BadRootKey) {
    namespace error = bklib::json::error;

    char const json_string[] = R"(
        {
            "bindingz": []
        }
    )";

    std::stringstream in {json_string};

    BK_ASSERT_THROWS_AND(tez::bindings_parser {in}, error::bad_index, {
        auto const index = boost::get_error_info<error::info_index>(e);
        ASSERT_NE(index, nullptr);

        utf8string string = boost::get<utf8string>(*index);
        ASSERT_STREQ("bindingz", string.c_str());
    });
}
