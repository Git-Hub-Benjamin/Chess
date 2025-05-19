#include "Chess/StandardLocalChessGame.hpp"
#include "Client/Options/Options.hpp"
#include "../../Util/Terminal/TextGraphics.hpp"
#include "Chess/Utils/ChessHelperFunctions.hpp"
#include "Chess/Utils/ChessConstants.hpp"
#include <cstdlib>
#include <chrono>
#include <codecvt>
#ifdef COMPILE_ONLINE
#include "../online-game/online-mode.hpp"
#endif
#include "../Options/Options.hpp"
#include "../ConfigFile/ConfigFile.hpp"
//! Always use wcout, wcout and cout dont mix

bool running = true;
#ifdef COMPILE_ONLINE
extern int online_game();
#endif
// GLOBAL
Options global_player_option;
bool SETTING_CHANGE_AFFECTS_CONFIG_FILE = true;
std::string CONFIG_PATH;



enum CHESS_CLOCK_SETUP_RET {
	CLOCK_SETUP,
	CLOCK_NONE,
	CLOCK_BACK
};

CHESS_CLOCK_SETUP_RET setup_chess_clock(ChessClock* obj) {
	configure_clock_screen();
	std::wcout << "--> ";
	int clockOption = get_menu_option();

	if (clockOption == GET_MENU_OPTION::ONE)
		return CLOCK_NONE;
	if (clockOption == GET_MENU_OPTION::BACK)
		return CLOCK_BACK;
	if (clockOption == GET_MENU_OPTION::TWO) {
		clock_presets_screen();
		std::wcout << "--> ";
		int preset_opt = get_menu_option();
		if (preset_opt == 4)
			return CLOCK_BACK;
		
		if (preset_opt == 1) 
			obj->initTime(THR_MIN, THR_MIN); // 180 seconds, 3 mins
		else if (preset_opt == 2)
			obj->initTime(FIVE_MIN, FIVE_MIN); // 300 seconds, 5 mins
		else if (preset_opt == 3)
			obj->initTime(FIF_TEEN_MIN, FIF_TEEN_MIN); // 300 seconds, 5 mins		
	} 
	if (clockOption == GET_MENU_OPTION::THREE) {
		std::wcout << "Unimplemented..." << std::endl;
		return CLOCK_BACK;
	}

	return CLOCK_SETUP;
}




namespace MAIN_MENU_SETTINGS_OPTIONS {
	enum MAIN_MENU_SETTINGS_OPTIONS {
		CHANGE_COLORS = 1,
		CHANGE_ART,
		MOVE_HIGHLIGHTING,
		DYNAMIC_MOVE_HIGHLIGHTING,
		BOARD_HISTORY,
		FLIP_ON_TURN,
		CLEAR_SCREEN_ON_PRINT,
		SETTINGS_AFFECT_CONFIG,
		CONFIG_FILE,
		BACK
	};
}

void changing_option_from_menu(int option_opt){
	bool back = false;
	int player_opt, color_opt;
	switch(option_opt){
		case MAIN_MENU_SETTINGS_OPTIONS::CHANGE_COLORS:
			if (!change_player_color_option())
				back = true;
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::CHANGE_ART:
			std::wcout << "1. Player one art\n2. Player two art\n3. Back\n--> ";
			player_opt = get_menu_option();
			if(player_opt == 1){
				std::wcout << "1. Unicode    " << TEXT_PIECE_ART_COLLECTION[0][1] << " | "; piece_art_option_active_inactive(STD_PIECE_ART_P1, player_opt); std::wcout << std::endl;
				std::wcout << "2. Characters P | "; piece_art_option_active_inactive(STD_PIECE_CHAR_P1, player_opt); std::wcout << std::endl;
				std::wcout << "3. Back\n--> ";
			} else if (player_opt == 2) {
				std::wcout << "1. Unicode    " << TEXT_PIECE_ART_COLLECTION[1][1] << " | "; piece_art_option_active_inactive(STD_PIECE_ART_P2, player_opt); std::wcout << std::endl;
				std::wcout << "2. Characters P | "; piece_art_option_active_inactive(STD_PIECE_CHAR_P2, player_opt); std::wcout << std::endl;
				std::wcout << "3. Back\n--> ";
			} else {
				back = true;
			}

			if (player_opt == 1 || player_opt == 2) {
				color_opt = get_menu_option();
				if(color_opt == 3){
					back = true;
				}else{
					
					enum TEXT_PIECE_ART_COLLECTION_SELECTOR* currPlayerArt = (player_opt == 1 ? &global_player_option.whitePlayerArtSelector : &global_player_option.blackPlayerArtSelector);
					switch(color_opt){
						case 1:
							if (player_opt == 1)
								*currPlayerArt = STD_PIECE_ART_P1;
							else 
								*currPlayerArt = STD_PIECE_ART_P2;
							break;
						case 2:
							if (player_opt == 1)
								*currPlayerArt = STD_PIECE_CHAR_P1;
							else 
								*currPlayerArt = STD_PIECE_CHAR_P2;
							break;
						default:
							back = true;
							break;
					}
				}
			}
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::MOVE_HIGHLIGHTING:
			global_player_option.moveHighlighting = !global_player_option.moveHighlighting;
			global_player_option.dynamicMoveHighlighting = false;
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::DYNAMIC_MOVE_HIGHLIGHTING:
			global_player_option.dynamicMoveHighlighting = !global_player_option.dynamicMoveHighlighting;
			global_player_option.moveHighlighting = false;
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::BOARD_HISTORY:
			global_player_option.boardHistory = !global_player_option.boardHistory;
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::FLIP_ON_TURN:
			global_player_option.flipBoardOnNewTurn = !global_player_option.flipBoardOnNewTurn;
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::CLEAR_SCREEN_ON_PRINT:
			global_player_option.clearScreenOnBoardPrint = !global_player_option.clearScreenOnBoardPrint;
			break;
		case MAIN_MENU_SETTINGS_OPTIONS::SETTINGS_AFFECT_CONFIG:
			SETTING_CHANGE_AFFECTS_CONFIG_FILE = !SETTING_CHANGE_AFFECTS_CONFIG_FILE;
			break;
		default:
			back = true;
			break;
	}

	if (global_player_option.moveHighlighting && global_player_option.dynamicMoveHighlighting)
		global_player_option.moveHighlighting = false; // Default to dynamic when trying to set both

	if(!back){
		if(SETTING_CHANGE_AFFECTS_CONFIG_FILE)
			overwrite_option_file();
	}
}



#ifdef COMPILE_ONLINE
extern JOIN_GAME_INFO createPrivateLobby2(int);
#endif
namespace MAIN_MENU_OPTIONS {
	enum MAIN_MENU_OPTIONS {
		DEV = -1,
		LOCAL_GAME = 1,
		ONLINE_GAME,
		OPTIONS,
		QUIT = 10
	};
}

int main() {	
	std::wcout << "IN PROGRAM" << std::endl;	
	// Set the global locale to support UTF-8 encoding

#ifdef __linux__
    std::locale::global(std::locale("en_US.UTF-8"));
#elif _WIN32
	std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);
    std::wcout.imbue(utf8_locale);
    std::wcin.imbue(utf8_locale);
#endif
	set_terminal_color(DEFAULT);

	std::wcout << "\n\n\n\n\n" << std::endl;
	
	// SETUP PATH FOR FOREVER USE
#ifdef __linux__
    std::string home = getenv("HOME");
#elif _WIN32
    std::string home = "";
    char* pValue = getenv("USERPROFILE"); // Use standard getenv
    if (pValue != nullptr) {
        home = std::string(pValue);
        // No need to free() for getenv, its memory is managed by the system
    }
#endif

	
	std::wcout << convertString(home) << std::endl;
	CONFIG_PATH = home + CONFIG_FILE_NAME;
	std::wcout << convertString(CONFIG_PATH) << std::endl;

	init_config_load();

	StandardLocalChessGame Game(global_player_option, ChessTypes::Player::PlayerOne, true);
	Game.startGame();

	while(running){
		title_screen();
		std::wcout << "Global things --> " << global_player_option.p1_color << global_player_option.p2_color << global_player_option.whitePlayerArtSelector << global_player_option.blackPlayerArtSelector << global_player_option.moveHighlighting << global_player_option.dynamicMoveHighlighting << global_player_option.boardHistory << global_player_option.flipBoardOnNewTurn << global_player_option.clearScreenOnBoardPrint << std::endl;

		std::wcout << "--> ";
		int mainMenuOption = get_menu_option();
		if(mainMenuOption <= MAIN_MENU_OPTIONS::LOCAL_GAME){ // Local Game or Dev Game
			local_game_screen();
			std::wcout << "--> ";
			int local_opt = get_menu_option(); // Standard Game or Quit
			if (local_opt != 2) {
				ChessClock chessclock;
				CHESS_CLOCK_SETUP_RET clockOption = setup_chess_clock(&chessclock);
				if (clockOption != CLOCK_BACK) {
					inital_turn_screen();
					std::wcout << "--> ";
					GET_MENU_OPTION turnOption = get_menu_option();
					if (turnOption != GET_MENU_OPTION::BACK) {
						if (clockOption == CLOCK_NONE) {
							StandardLocalChessGame Game(global_player_option, static_cast<ChessTypes::Player>(turnOption == GET_MENU_OPTION::THREE ? 0 : turnOption), mainMenuOption == MAIN_MENU_OPTIONS::DEV ? true : false);
							Game.startGame();
						} else {
							StandardLocalChessGame Game(global_player_option, chessclock, static_cast<ChessTypes::Player>(turnOption == GET_MENU_OPTION::THREE ? 0 : turnOption), mainMenuOption == MAIN_MENU_OPTIONS::DEV ? true : false);
							Game.startGame();
						}
					}
				}
			} 
			
		}else if(mainMenuOption == MAIN_MENU_OPTIONS::ONLINE_GAME){
#ifdef COMPILE_ONLINE
			online_game();
#else
			continue;
#endif
		}else if(mainMenuOption == MAIN_MENU_OPTIONS::OPTIONS){
			option_screen();
			GET_MENU_OPTION option_opt = get_menu_option();
			if(option_opt == static_cast<GET_MENU_OPTION>(MAIN_MENU_SETTINGS_OPTIONS::CONFIG_FILE)) // For config file options
				config_file_options();
			if(option_opt != static_cast<GET_MENU_OPTION>(MAIN_MENU_SETTINGS_OPTIONS::BACK)) // Handle anything else, if 7 then just go back
				changing_option_from_menu(option_opt);	
		}else if(mainMenuOption == MAIN_MENU_OPTIONS::QUIT){
			running = false;
			std::wcout << "Have a good day..." << std::endl;
		}
		// if anything else just run again
	}
	return 0;
}
