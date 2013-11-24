#include "pch.hpp"

#include <gtest/gtest.h>
#include "keyboard.hpp"

#include "game/hotkeys.hpp"

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

using namespace tez;
//------------------------------------------------------------------------------
TEST(Hotkeys, Combo) {
    using namespace tez;

    bklib::key_combo combo {bklib::keys::LEFT};

    auto command = hotkeys::command_from_combo(combo);

    ASSERT_EQ(command, tez::game_command::DIR_WEST);
}
//------------------------------------------------------------------------------
#define ASSERT_THROW_AND(expression, exception, handler)\
[&] {\
    try {\
        expression;\
    } catch (exception const& e) {\
        handler(e);\
    } catch (...) {\
        ASSERT_FALSE("unexpected exception.");\
    }\
}()

//------------------------------------------------------------------------------
TEST(Hotkeys, BadRoot) {
    utf8string string = {R"({
        "bindingz": ""
    })"};

    std::stringstream stream {string};

    auto const handler = [](json::error::bad_index const& e) {
        json::error::dump_exception(e);

        auto const expected = boost::get_error_info<json::error::info_expected_type>(e);
        auto const actual   = boost::get_error_info<json::error::info_actual_type>(e);

        ASSERT_NE(expected, nullptr);
        ASSERT_NE(actual, nullptr);

        ASSERT_EQ(*expected, "array");
        ASSERT_EQ(*actual, "null");
    };

    ASSERT_THROW_AND(hotkeys::reload(stream), json::error::bad_index, handler);
}
//------------------------------------------------------------------------------
TEST(Hotkeys, BadBindingType) {
    utf8string string = {R"({
        "bindings": [
            "blah"
        ]
    })"};

    std::stringstream stream {string};

    auto const handler = [](json::error::bad_type const& e) {
        auto const expected = boost::get_error_info<json::error::info_expected_type>(e);
        auto const actual   = boost::get_error_info<json::error::info_actual_type>(e);

        ASSERT_NE(expected, nullptr);
        ASSERT_NE(actual, nullptr);

        ASSERT_EQ(*expected, "array");
        ASSERT_EQ(*actual, "string");
    };

    ASSERT_THROW_AND(hotkeys::reload(stream), json::error::bad_type, handler);
}
//------------------------------------------------------------------------------
TEST(Hotkeys, BadBindingSize) {
    utf8string string = {R"({
        "bindings": [
            ["a"]
        ]
    })"};

    std::stringstream stream {string};

    auto const handler = [](json::error::bad_size const& e) {
        auto const expected = boost::get_error_info<json::error::info_expected_size>(e);
        auto const actual   = boost::get_error_info<json::error::info_actual_size>(e);

        ASSERT_NE(expected, nullptr);
        ASSERT_NE(actual, nullptr);

        ASSERT_EQ(*expected, 2);
        ASSERT_EQ(*actual, 1);
    };

    ASSERT_THROW_AND(hotkeys::reload(stream), json::error::bad_size, handler);
}
//------------------------------------------------------------------------------
TEST(Hotkeys, BadBindingSubType1) {
    utf8string string = {R"({
        "bindings": [
            [1, "b"]
        ]
    })"};

    std::stringstream stream {string};

    auto const handler = [](json::error::bad_type const& e) {
        auto const expected = boost::get_error_info<json::error::info_expected_type>(e);
        auto const actual   = boost::get_error_info<json::error::info_actual_type>(e);

        ASSERT_NE(expected, nullptr);
        ASSERT_NE(actual, nullptr);

        ASSERT_EQ(*expected, "string");
        ASSERT_EQ(*actual, "int");
    };

    ASSERT_THROW_AND(hotkeys::reload(stream), json::error::bad_type, handler);
}
//------------------------------------------------------------------------------
TEST(Hotkeys, BadBindingSubType2) {
    utf8string string = {R"({
        "bindings": [
            ["cmd", "b"]
        ]
    })"};

    std::stringstream stream {string};

    auto const handler = [](json::error::bad_type const& e) {
        auto const expected = boost::get_error_info<json::error::info_expected_type>(e);
        auto const actual   = boost::get_error_info<json::error::info_actual_type>(e);

        ASSERT_NE(expected, nullptr);
        ASSERT_NE(actual, nullptr);

        ASSERT_EQ(*expected, "array");
        ASSERT_EQ(*actual, "string");
    };

    ASSERT_THROW_AND(hotkeys::reload(stream), json::error::bad_type, handler);
}
