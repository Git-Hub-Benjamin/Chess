#pragma once
#include <iostream>

extern std::string CONFIG_PATH;

enum GET_MENU_OPTION {
    DEV = -1,
    ONE = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    BACK,
    INVALID
};

#define THR_MIN 60 * 3
#define FIVE_MIN 60 * 3
#define FIF_TEEN_MIN 60 * 3

// false (BACK BUTTON)
// true (good)
bool change_player_color_option();
void overwrite_option_file();
bool erase_config_file();
enum GET_MENU_OPTION get_menu_option();
