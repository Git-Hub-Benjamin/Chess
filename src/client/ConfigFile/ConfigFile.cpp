#include "ConfigFile.hpp"

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
    std::ofstream file(CONFIG_PATH); 
    if(!file.is_open())
        return false;
    file.close();
    return true;
}

void default_setting_config(){
    std::ofstream file(CONFIG_PATH); 
    if(!file.is_open())
        std::wcout << "Error opening file for writing default settings." << std::endl;
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
        file.close(); 
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