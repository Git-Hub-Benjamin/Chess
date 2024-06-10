#include "../Chess/chess.hpp"
#include "./option.hpp"
#include "./client-text-graphics/textgraphic.hpp"
#include <cstdlib>
#include <fstream>	
#include <chrono>
//! Always use wcout, wcout and wcout dont mix

bool running = true;
extern void online_game();

// GLOBAL
Options global_player_option;
bool SETTING_CHANGE_AFFECTS_CONFIG_FILE = true;

// STATIC
std::string CONFIG_PATH;

int get_menu_option(){
	while(true){
		std::wstring str;
		std::wcin >> str;
		if(str[0] == L'1')
			return 1;
		if(str[0] == L'2')
			return 2;
		if(str[0] == L'3')
			return 3;
		if(str[0] == L'4')
			return 4;
		if(str[0] == L'5')
			return 5;
		if(str[0] == L'6')
			return 6;
		if(str[0] == L'7')
			return 7;
		if(str[0] == L'8')
			return 8;
		if(str[0] == L'D' || str[0] == L'd')
			return -1;
		break;
	}
	return -1;
}

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
	int opt = get_menu_option();

	if (opt == 1)
		return CLOCK_NONE;
	if (opt == 4)
		return CLOCK_BACK;
	if (opt == 2) {
		clock_presets_screen();
		std::wcout << "--> ";
		int preset_opt = get_menu_option();
		if (preset_opt == 4)
			return CLOCK_BACK;
		
		if (preset_opt == 1) 
			obj->initTime(60 * 3, 60 * 3); // 180 seconds, 3 mins
		else if (preset_opt == 2)
			obj->initTime(60 * 5, 60 * 5); // 300 seconds, 5 mins
		else if (preset_opt == 3)
			obj->initTime(60 * 15, 60 * 15); // 300 seconds, 5 mins		
	} 
	if (opt == 3) {
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
		}else if(option.compare("board_history") == 0){
			superTemp.boardHistory = val == 0 ? false : true;
		}else if(option.compare("flip_on_turn") == 0){
			superTemp.flipBoardOnNewTurn = val == 0 ? false : true;
		}else{
			// Tampered, unknown option
			res = TAMPERED;
			goto out;
		}
	}

	
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

bool erase_config_file(){
	std::ofstream file(CONFIG_PATH);
	if(!file.is_open())
		return false;
	
	file.close();
	std::ofstream erase(CONFIG_PATH, std::fstream::out | std::fstream::trunc );
	erase.close();
	return true;

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
		"move_highlighting:" << global_player_option.moveHighlighting << "\n"
		"board_history:" << global_player_option.boardHistory << "\n" << 
		"flip_on_turn:" << global_player_option.flipBoardOnNewTurn << "\n";
	}
}

void changing_option_from_menu(int option_opt){
	bool back = false;
	int player_opt, color_opt;
	switch(option_opt){
		case 1:
			// ask which player to change and what color
			std::wcout << "1. Player one color\n2. Player two color\n3. Back\n--> ";
			player_opt = get_menu_option();
			if(player_opt == 1 || player_opt == 2){
				std::wcout << "1. Default (Thin White) | "; color_option_active_inactive(DEFAULT, player_opt); std::wcout << std::endl;
				std::wcout << "2. Bold white 	        | "; color_option_active_inactive(BOLD, player_opt); std::wcout << std::endl;
				std::wcout << "3. Blue	                | "; color_option_active_inactive(BLUE, player_opt); std::wcout << std::endl;
				std::wcout << "4. Aqua 	        | "; color_option_active_inactive(AQUA, player_opt); std::wcout << std::endl;
				std::wcout << "5. Green                | "; color_option_active_inactive(GREEN, player_opt); std::wcout << std::endl;
				std::wcout << "6. Back\n--> ";
				color_opt = get_menu_option();
				if(color_opt == 6){
					back = true;
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
							*currPlayChangingColor = AQUA;
							break;
						case 5:
							*currPlayChangingColor = GREEN;
							break;
						default:
							back = true;
							break;
					}
					std::wcout << "After -> " << (player_opt == 1 ? global_player_option.p1_color : global_player_option.p2_color) << std::endl;
				}
			}else
				back = true;
			break;
		case 2:
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
		case 3:
			global_player_option.moveHighlighting = !global_player_option.moveHighlighting;
			break;
		case 4:
			global_player_option.boardHistory = !global_player_option.boardHistory;
			break;
		case 5:
			global_player_option.flipBoardOnNewTurn = !global_player_option.flipBoardOnNewTurn;
			break;
		case 6:
			SETTING_CHANGE_AFFECTS_CONFIG_FILE = !SETTING_CHANGE_AFFECTS_CONFIG_FILE;
			break;
		default:
			back = true;
			break;
	}
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
		"p2_art:" << 1 << "\n" <<
		"move_highlighting:" << 0 << "\n"
		"board_history:" << 0 << "\n" << 
		"flip_on_turn:" << 0 << "\n";
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

int main()
{		
	// Set the global locale to support UTF-8 encoding
    std::locale::global(std::locale("en_US.UTF-8"));
	set_terminal_color(DEFAULT);

	std::wcout << "\n\n\n\n\n" << std::endl;

	// ANNOYING I HAVE TO DO THIS
	bool firstPrint = true;
	
	// SETUP PATH FOR FOREVER USE
	std::string home = getenv("HOME");
	std::string path = home + "/wchesscfg"; 
	CONFIG_PATH = home + CONFIG_FILE_NAME;


	while(running){
		title_screen();
		if(firstPrint){
			init_config_load();
			firstPrint = !firstPrint;		
		}
		std::wcout << "Global things --> " << global_player_option.p1_color << global_player_option.p2_color << global_player_option.whitePlayerArtSelector << global_player_option.blackPlayerArtSelector << global_player_option.moveHighlighting << global_player_option.boardHistory << global_player_option.flipBoardOnNewTurn << std::endl;

		std::wcout << "--> ";
		int opt = get_menu_option();
		if(opt == 1 || opt == -1){ // Local Game or Dev Game
			local_game_screen();
			std::wcout << "--> ";
			int local_opt = get_menu_option(); // Standard Game or Quit
			if (local_opt != 2) {
				ChessClock chessclock;
				CHESS_CLOCK_SETUP_RET res = setup_chess_clock(&chessclock);
				if (res != CLOCK_BACK) {
					if (local_opt == 1) {
						if (res == CLOCK_NONE) {
							Standard_ChessGame Game(global_player_option, opt == 7 ? true : false);
							Game.startGame();
						} else {
							Standard_ChessGame Game(global_player_option, chessclock, opt == 7 ? true : false);
							Game.startGame();
						}
					}
				}
			} 
			
		}else if(opt == 2){
			online_game();
		}else if(opt ==3){
			option_screen();
			int option_opt = get_menu_option();
			if(option_opt == 7) // For config file options
				config_file_options();
			if(option_opt != 8) // Handle anything else, if 7 then just go back
				changing_option_from_menu(option_opt);	
		}else if(opt == 4){
			running = false;
			std::wcout << "Have a good day..." << std::endl;
		}
		// if anything else just run again
	}
	return 0;
}


