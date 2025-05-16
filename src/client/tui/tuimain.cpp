#include "../../Chess/chess.hpp"
#include "../option.hpp"
#include "../client-text-graphics/textgraphic.hpp"
#include <cstdlib>
#include <fstream>	
#include <chrono>
#ifdef COMPILE_ONLINE
#include "../online-game/online-mode.hpp"
#endif
#include "./clientoption.hpp"
//! Always use wcout, wcout and cout dont mix

bool running = true;
#ifdef COMPILE_ONLINE
extern int online_game();
#endif
// GLOBAL
Options global_player_option;
bool SETTING_CHANGE_AFFECTS_CONFIG_FILE = true;
std::string CONFIG_PATH;

enum CFG_FILE_RET{
	FOUND,
	NOTFOUND,
	TAMPERED,
	FOUND_BUT_EMPTY
};

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


// True --> Found
// False --> Not found
enum CFG_FILE_RET load_config_file(Options& temp){

	std::ifstream file(CONFIG_PATH);

	// File exsists
	std::string line;
	Options superTemp; // if this is file is tampered we want to send back the defualt settings
	enum CFG_FILE_RET res = FOUND;
	bool notEmpty = false;

	while(std::getline(file, line)){
		notEmpty = true;
		int index = 0;
		index = line.find_first_of(":");
		if(index == 0 || index + 1 == line.length()){
			// option file has been tampered with
			res = TAMPERED;
			goto out;
		}

		int val = char_single_digit_to_int(line[index + 1]);
		std::string option = line.substr(0, index);
		if(option.compare("p1_color") == 0){
			superTemp.p1_color = (enum WRITE_COLOR)val;
		}else if(option.compare("p2_color") == 0){
			superTemp.p2_color = (enum WRITE_COLOR)val;
		}else if(option.compare("p1_art") == 0){
			superTemp.whitePlayerArtSelector = static_cast<TEXT_PIECE_ART_COLLECTION_SELECTOR>(val);
		}else if(option.compare("p2_art") == 0){
			superTemp.blackPlayerArtSelector = static_cast<TEXT_PIECE_ART_COLLECTION_SELECTOR>(val);
		}else if(option.compare("move_highlighting") == 0){
			superTemp.moveHighlighting = val == 0 ? false : true;
		}else if (option.compare("dynamic_move_highlighting") == 0){
			superTemp.dynamicMoveHighlighting = val == 0 ? false : true;
		}else if(option.compare("board_history") == 0){
			superTemp.boardHistory = val == 0 ? false : true;
		}else if(option.compare("flip_on_turn") == 0){
			superTemp.flipBoardOnNewTurn = val == 0 ? false : true;
		} else if (option.compare("print_clear_screen") == 0){
			superTemp.clearScreenOnBoardPrint = val == 0 ? false : true;
		}else{
			// Tampered, unknown option
			res = TAMPERED;
			goto out;
		}
	}

	if (superTemp.moveHighlighting && superTemp.dynamicMoveHighlighting)
		superTemp.moveHighlighting = false; // Default to dynamic if both are active
	
out:
	file.close(); // Always close file
	if(res != TAMPERED){
		temp = superTemp;
		return notEmpty ? FOUND : FOUND_BUT_EMPTY;
	}else
		return res;
}

enum CFG_FILE_RET locate_config_file(){
	std::ifstream file(CONFIG_PATH);
	if(!file.is_open())
		return NOTFOUND;
	return FOUND;
}

bool create_new_config_file(){
	std::fstream file(CONFIG_PATH);
	if(!file.is_open())
		return false;
	return true;
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

void default_setting_config(){
	std::fstream file(CONFIG_PATH);
	if(!file.is_open())
		std::wcout << "Error opening file." << std::endl;
	else{
		file << "p1_color:" << 0 << "\n" << 
		"p2_color:" << 0 << "\n" <<
		"p1_art:" << 0 << "\n" <<
		"p2_art:" << 1 << "\n" << // default is 1
		"move_highlighting:" << 0 << "\n" << 
		"dynamic_move_highlighting:" << 0 << "\n" <<
		"board_history:" << 0 << "\n" << 
		"flip_on_turn:" << 0 << "\n" <<
		"print_clear_screen:" << 0 << "\n"; 
	}
}

//! FOR NEXT TIME, IMPORTANT

//! 1. CANT/ DONT OPEN A INPUT AND OUTPUT STREAM FOR THE SAME FILE AT THE SAME TIME
//! 2. MAKE MAKE MAKE SURE YOUR CLOSE FILES WHEN YOU OPEN THEM
//! 3. DONT OPEN TWO STREAMS FOR THE SAME FILE AT ONCE, ONLY ONE AT A TIME

void init_config_load(){
	Options temp;
	std::wstring msg;

	if(locate_config_file() == FOUND){
		enum CFG_FILE_RET loadRet = load_config_file(temp);
		// Could return, FOUND, TAMPERED, OR FOUND_BUT_EMPTY
		if(loadRet == FOUND){
			// Settings found in config file, lets set the global settings to these ones
			global_player_option = temp;
			std::wcout << "Config file successfully loaded." << std::endl; 
		}else if(loadRet == TAMPERED){
			std::wcout << "Config file tampered, Make new? (Y/y) \n-->";
			std::wcin >> msg;
			if(msg[0] == L'Y' || msg[0] == L'y'){
				msg = L"";
				if(erase_config_file())
					std::wcout << "Erased file." << std::endl;
				else
					std::wcout << "Error opening file." << std::endl;
		
				default_setting_config();
				std::wcout << "Default settings applied to config file." << std::endl;
			}
		}else{
			// FOUND_BUT_EMPTY
			default_setting_config();
			std::wcout << "Default settings applied to config file." << std::endl;
		}

	}else{
		// Not found
		if(create_new_config_file()){
			std::wcout << "Created new config file." << std::endl;
			default_setting_config();
		}else
			std::wcout << "Error creating new config file." << std::endl;
	}
}

void config_file_options(){
	std::wcout << "\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
	std::wcout << "1. Re-load\n2. Reset to default\n3. Back";
	std::wcout << "\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =\n\n-->";
	int config_opt = get_menu_option();
	if(config_opt == 1){
		init_config_load();
	}else if(config_opt == 2){
		erase_config_file();
		default_setting_config();
		std::wcout << "Default settings applied to config file." << std::endl;
		init_config_load();
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
		QUIT
	};
}

int main()
{	
	std::wcout << "IN PROGRAM" << std::endl;	
	// Set the global locale to support UTF-8 encoding

#ifdef __linux__
    std::locale::global(std::locale("en_US.UTF-8"));
#elif _WIN32
	std::locale::global(std::locale(""));
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
    // Note: getenv is not thread-safe and doesn't provide size info like _dupenv_s.
    // For a more robust solution, you might need to handle thread-safety or
    // use a different Windows API function if you need the size or a thread-safe version.
    // For simply getting the home directory, getenv is often sufficient.
#endif


	std::string path = home + "/wchesscfg"; 
	CONFIG_PATH = home + CONFIG_FILE_NAME;
	std::cout << CONFIG_PATH << std::endl;

	init_config_load();

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
							StandardLocalChessGame Game(global_player_option, static_cast<Player>(turnOption == GET_MENU_OPTION::THREE ? 0 : turnOption), mainMenuOption == MAIN_MENU_OPTIONS::DEV ? true : false);
							Game.startGame();
						} else {
							StandardLocalChessGame Game(global_player_option, chessclock, static_cast<Player>(turnOption == GET_MENU_OPTION::THREE ? 0 : turnOption), mainMenuOption == MAIN_MENU_OPTIONS::DEV ? true : false);
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


