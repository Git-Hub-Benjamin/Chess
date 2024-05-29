#pragma once

#include <string>
#include <iostream>
#include "../option.hpp"
#include "../../terminal-io/terminal.hpp"

extern Options global_player_option;
extern bool SETTING_CHANGE_AFFECTS_CONFIG_FILE;

void title_screen();
void option_screen();
void online_menu(std::wstring);
void display_rand_queue_menu();
void enter_private_lobby_code_menu();
void display_private_lobby_code_menu(std::wstring& str);
void color_option_active_inactive(enum WRITE_COLOR check, int playerNum);