#include "pch.hpp"

#include <gtest/gtest.h>
#include "keyboard.hpp"

#include "game/hotkeys.hpp"

TEST(Keyboard, Mappings) {
    using namespace bklib;

    auto name = keyboard::key_name(keys::UP);
    auto key  = keyboard::key_code(name);

    ASSERT_EQ(keys::UP, key);
}

TEST(Keyboard, SetGet) {
    using namespace bklib;

    keyboard kb;

    int clear_count = 0;
    auto on_clear = [&](keyboard& state, keys k) {
        clear_count++;
    };

    for (uint8_t i = 0; i < 0xFF; ++i) {
        auto const key = static_cast<keys>(i);

        ASSERT_FALSE(kb[key].is_down);
        kb.set_state(key, true);
        ASSERT_TRUE(kb[key].is_down);
    }

    kb.clear(on_clear);
    ASSERT_EQ(clear_count, 0xFF);
}


TEST(Keyboard, Combo) {
    using namespace bklib;

    key_combo combo1 = {keys::SHIFT_R, keys::SPACE};
    ASSERT_EQ(combo1.size(), 2);
    ASSERT_FALSE(combo1.empty());

    ASSERT_FALSE(combo1.remove(keys::SHIFT_L));

    ASSERT_TRUE(combo1.add(keys::CTRL_L));
    ASSERT_EQ(combo1.size(), std::distance(combo1.begin(), combo1.end()));
    ASSERT_EQ(combo1.size(), 3);

    key_combo combo2;
    ASSERT_TRUE(combo2.empty());
    ASSERT_EQ(combo2.size(), std::distance(combo2.begin(), combo2.end()));
    ASSERT_EQ(combo2.size(), 0);

    combo2.add(keys::CTRL_L);
    combo2.add(keys::SHIFT_R);
    combo2.add(keys::SPACE);
    ASSERT_EQ(combo2.size(), 3);

    ASSERT_EQ(combo1, combo2);
    ASSERT_EQ(combo2, combo1);

    key_combo combo3;
    combo3.add({keys::CTRL_L, keys::SHIFT_R, keys::SPACE});

    ASSERT_EQ(combo1, combo3);
    ASSERT_EQ(combo2, combo3);
    ASSERT_EQ(combo3, combo3);
}

TEST(Hotkeys, Combo) {
    using namespace tez;

    bklib::key_combo combo {bklib::keys::LEFT};

    auto command = hotkeys::translate(combo);

    ASSERT_EQ(command, tez::game_command::DIR_WEST);
}
