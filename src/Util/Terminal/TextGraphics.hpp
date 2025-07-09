#pragma once

#include <string>
#include <iostream>
#include "Terminal.hpp"
#include "../../Client/Options/Options.hpp"

extern Options global_player_option;
extern bool SETTING_CHANGE_AFFECTS_CONFIG_FILE;

void title_screen();
void option_screen();
void local_game_screen();
void inital_turn_screen();
void clock_presets_screen();
void configure_clock_screen();
void online_menu(std::wstring);
void display_rand_queue_menu();
void enter_private_lobby_code_menu();
void displayPrivateLobbyCodeMenu(std::wstring& str);
void color_option_active_inactive(enum WRITE_COLOR check, int playerNum);
void piece_art_option_active_inactive(enum TEXT_PIECE_ART_COLLECTION_SELECTOR, int);
