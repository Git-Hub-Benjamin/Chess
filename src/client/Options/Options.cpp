#include "Options.hpp"
#include "../../Util/Terminal/TextGraphics.hpp"
#include <fstream>

GET_MENU_OPTION get_menu_option(){
    std::wstring str;
    std::wcin >> str;
    if(str[0] == L'1')
        return GET_MENU_OPTION::ONE;
    if(str[0] == L'2')
        return GET_MENU_OPTION::TWO;
    if(str[0] == L'3')
        return GET_MENU_OPTION::THREE;
    if(str[0] == L'4')
        return GET_MENU_OPTION::FOUR;
    if(str[0] == L'5')
        return GET_MENU_OPTION::FIVE;
    if(str[0] == L'6')
        return GET_MENU_OPTION::SIX;
    if(str[0] == L'7')
        return GET_MENU_OPTION::SEVEN;
    if(str[0] == L'8')
        return GET_MENU_OPTION::EIGHT;
    if(str[0] == L'9')
        return GET_MENU_OPTION::NINE;
    if(str[0] == L'D' || str[0] == L'd')
        return GET_MENU_OPTION::DEV;
    if(str[0] == L'X' || str[0] == L'x')
        return GET_MENU_OPTION::BACK;
	return GET_MENU_OPTION::INVALID;
}

bool erase_config_file(){
#ifdef __linux__
	std::ofstream file(CONFIG_PATH);
	if(!file.is_open())
		return false;
	
	file.close();
	std::ofstream erase(CONFIG_PATH, std::fstream::out | std::fstream::trunc );
	erase.close();
	return true;
#else
// On Windows, you can use the remove function to delete a file
    if (remove(CONFIG_PATH.c_str()) != 0) {
        // Error removing file
        return false;
    }
    return true;
#endif
}

void overwrite_option_file(){
	std::ofstream file(CONFIG_PATH);
	if(!file.is_open())
		std::wcout << "Problem locating config file, go to options for help.\nIf problem persists, change to affect local only for temporary fix..." << std::endl;
	else{
		erase_config_file();
		file << "p1_color:" << global_player_option.p1_color << "\n" << 
		"p2_color:" << global_player_option.p2_color << "\n" <<
		"p1_art:" << global_player_option.whitePlayerArtSelector << "\n" <<
		"p2_art:" << global_player_option.blackPlayerArtSelector << "\n" <<
		"move_highlighting:" << global_player_option.moveHighlighting << "\n" <<
		"dynamic_move_highlighting:" << global_player_option.dynamicMoveHighlighting << "\n" <<
		"board_history:" << global_player_option.boardHistory << "\n" << 
		"flip_on_turn:" << global_player_option.flipBoardOnNewTurn << "\n" <<
        "print_clear_screen:" << global_player_option.clearScreenOnPrint << "\n";
	}
}

// false (BACK BUTTON)
// true (good)
bool change_player_color_option() {
    int player_opt, color_opt;
    // ask which player to change and what color
    std::wcout << "1. Player one color\n2. Player two color\n3. Back\n--> ";
    player_opt = get_menu_option();
    if(player_opt == 1 || player_opt == 2){
        std::wcout << "1. Default (Thin White) | "; color_option_active_inactive(DEFAULT, player_opt); std::wcout << std::endl;
        std::wcout << "2. Bold white 	        | "; color_option_active_inactive(BOLD, player_opt); std::wcout << std::endl;
        std::wcout << "3. Blue	                | "; color_option_active_inactive(BLUE, player_opt); std::wcout << std::endl;
        std::wcout << "4. Yellow 	      | "; color_option_active_inactive(YELLOW, player_opt); std::wcout << std::endl;
        std::wcout << "5. Green                | "; color_option_active_inactive(GREEN, player_opt); std::wcout << std::endl;
        std::wcout << "6. Back\n--> ";
        color_opt = get_menu_option();
        if(color_opt == 6){
            return false;
        }else{
            std::wcout << "Player " << player_opt << ", is changing their color" << std::endl;
            std::wcout << "Before -> " << (player_opt == 1 ? global_player_option.p1_color : global_player_option.p2_color) << std::endl;
            std::wcout << "Address of color in global --> " << (player_opt == 1 ? &global_player_option.p1_color : &global_player_option.p2_color) << std::endl;
            enum WRITE_COLOR* currPlayChangingColor = (player_opt == 1 ? &global_player_option.p1_color : &global_player_option.p2_color);
            std::wcout << "Address of color in pointer --> " << currPlayChangingColor << std::endl;
            switch(color_opt){
                case 1:
                    *currPlayChangingColor = DEFAULT;
                    break;
                case 2:
                    *currPlayChangingColor = BOLD;
                    break;
                case 3:
                    *currPlayChangingColor = BLUE;
                    break;
                case 4:
                    *currPlayChangingColor = YELLOW;
                    break;
                case 5:
                    *currPlayChangingColor = GREEN;
                    break;
                default:
                    return false;
                    break;
            }
            std::wcout << "After -> " << (player_opt == 1 ? global_player_option.p1_color : global_player_option.p2_color) << std::endl;
            return true;
        }
    }else
        return false;
}