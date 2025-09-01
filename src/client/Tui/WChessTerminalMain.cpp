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
#include "../../Util/Terminal/TextPieceArt.hpp"

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
	WChessPrint("--> ");
	int clockOption = get_menu_option();

	if (clockOption == GET_MENU_OPTION::ONE)
		return CLOCK_NONE;
	if (clockOption == GET_MENU_OPTION::BACK)
		return CLOCK_BACK;
	if (clockOption == GET_MENU_OPTION::TWO) {
		clock_presets_screen();
		WChessPrint("--> ");
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
		WChessPrint("Unimplemented...\n");
		return CLOCK_BACK;
	}

	return CLOCK_SETUP;
}




enum class TerminalSettingsOptions {
	ChangeColors = 1,
	ChangeArt,
	MoveHighlighting,
	DynamicMoveHighlighting,
	BoardHistory,
	FlipOnTurn,
	ClearScreenOnPrint,
	SettingsAffectConfig,
	ConfigFile,
	Back
};

void changing_option_from_menu(TerminalSettingsOptions option_opt){
	bool back = false;
	int player_opt, color_opt;
	switch(option_opt){
		case TerminalSettingsOptions::ChangeColors:
			if (!change_player_color_option())
				back = true;
			break;
		case TerminalSettingsOptions::ChangeArt:
			WChessPrint("1. Player one art\n2. Player two art\n3. Back\n--> ");
			player_opt = get_menu_option();
			if(player_opt == 1){
				WChessPrint("1. Unicode    "); WChessPrint(TEXT_PIECE_ART_COLLECTION[0][1]); WChessPrint(" | ");
				piece_art_option_active_inactive(STD_PIECE_ART_P1, player_opt);
				WChessPrint("\n");
				WChessPrint("2. Characters P | ");
				piece_art_option_active_inactive(STD_PIECE_CHAR_P1, player_opt);
				WChessPrint("\n");
				WChessPrint("3. Back\n--> ");
			} else if (player_opt == 2) {
				WChessPrint("1. Unicode    "); WChessPrint(TEXT_PIECE_ART_COLLECTION[1][1]); WChessPrint(" | ");
				piece_art_option_active_inactive(STD_PIECE_ART_P2, player_opt);
				WChessPrint("\n");
				WChessPrint("2. Characters P | ");
				piece_art_option_active_inactive(STD_PIECE_CHAR_P2, player_opt);
				WChessPrint("\n");
				WChessPrint("3. Back\n--> ");
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
		case TerminalSettingsOptions::MoveHighlighting:
			global_player_option.moveHighlighting = !global_player_option.moveHighlighting;
			global_player_option.dynamicMoveHighlighting = false;
			break;
		case TerminalSettingsOptions::DynamicMoveHighlighting:
			global_player_option.dynamicMoveHighlighting = !global_player_option.dynamicMoveHighlighting;
			global_player_option.moveHighlighting = false;
			break;
		case TerminalSettingsOptions::BoardHistory:
			global_player_option.boardHistory = !global_player_option.boardHistory;
			break;
		case TerminalSettingsOptions::FlipOnTurn:
			global_player_option.flipBoardOnNewTurn = !global_player_option.flipBoardOnNewTurn;
			break;
		case TerminalSettingsOptions::ClearScreenOnPrint:
			global_player_option.clearScreenOnPrint = !global_player_option.clearScreenOnPrint;
			break;
		case TerminalSettingsOptions::SettingsAffectConfig:
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
enum class TerminalMainMenuOptions {
	Dev = -1,
	LocalGame = 1,
	OnlineGame,
	Options,
	Quit = 10
};

int main() {	
#ifdef __linux__
    std::locale::global(std::locale("en_US.UTF-8"));
#elif _WIN32 //! trying w/o if it doesnt work add it back
	// std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);
    // std::wcout.imbue(utf8_locale);
    // std::wcin.imbue(utf8_locale);
#endif
	setTerminalColor(DEFAULT);

#ifdef LEGACY_ARRAY_GAMEBOARD
	WChessPrint("Legacy array gameboard selected\n");
#else
	WChessPrint("Bitboard gameboard selected\n");
#endif

	WChessPrint("\n\n\n\n\n\n");

	WChessPrint("Welcome to WChess!\n");
	
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
	
	CONFIG_PATH = home + CONFIG_FILE_NAME;
	init_config_load();

	while(running){
		title_screen();
		WChessPrint("Global things --> "); 

		std::string globals = std::to_string(global_player_option.p1_color) + std::to_string(global_player_option.p2_color) + std::to_string(global_player_option.whitePlayerArtSelector) + std::to_string(global_player_option.blackPlayerArtSelector) + std::to_string(global_player_option.moveHighlighting) + std::to_string(global_player_option.dynamicMoveHighlighting) + std::to_string(global_player_option.boardHistory) + std::to_string(global_player_option.flipBoardOnNewTurn) + std::to_string(global_player_option.clearScreenOnPrint);
		WChessPrint(globals.c_str());
		WChessPrint("\n");

		WChessPrint("--> ");
		int mainMenuOption = get_menu_option();
		if(mainMenuOption == static_cast<int>(TerminalMainMenuOptions::LocalGame) || mainMenuOption == static_cast<int>(TerminalMainMenuOptions::Dev)){ // Local Game or Dev Game
			local_game_screen();
			WChessPrint("--> ");
			int local_opt = get_menu_option(); // Standard Game or Quit
			if (local_opt != 2) {
				ChessClock chessclock;
				CHESS_CLOCK_SETUP_RET clockOption = setup_chess_clock(&chessclock);
				if (clockOption != CLOCK_BACK) {
					inital_turn_screen();
					WChessPrint("--> ");
					GET_MENU_OPTION turnOption = get_menu_option();
					if (turnOption != GET_MENU_OPTION::BACK) {
						if (clockOption == CLOCK_NONE) {
							StandardLocalChessGame Game(
								global_player_option, 
								static_cast<ChessTypes::Player>(turnOption == GET_MENU_OPTION::THREE ? 0 : turnOption), 
								mainMenuOption == static_cast<int>(TerminalMainMenuOptions::Dev) ? true : false);
							Game.startGame();
						} else {
							StandardLocalChessGame Game(
								global_player_option, 
								chessclock, 
								static_cast<ChessTypes::Player>(turnOption == GET_MENU_OPTION::THREE ? 0 : turnOption), 
								mainMenuOption == static_cast<int>(TerminalMainMenuOptions::Dev) ? true : false);
							Game.startGame();
						}
					}
				}
			} 
			
		}else if(mainMenuOption == static_cast<int>(TerminalMainMenuOptions::OnlineGame)){
#ifdef COMPILE_ONLINE
			online_game();
#else
			continue;
#endif
		}else if(mainMenuOption == static_cast<int>(TerminalMainMenuOptions::Options)){
			option_screen();
			GET_MENU_OPTION option_opt = get_menu_option();
			if(option_opt == static_cast<GET_MENU_OPTION>(TerminalSettingsOptions::ConfigFile)) // For config file options
				config_file_options();
			if(option_opt != static_cast<GET_MENU_OPTION>(TerminalSettingsOptions::Back)) // Handle anything else, if 7 then just go back
				changing_option_from_menu(static_cast<TerminalSettingsOptions>(option_opt));	
		}else if(mainMenuOption == static_cast<int>(TerminalMainMenuOptions::Quit)){
			running = false;
			WChessPrint("Have a good day...\n");
		}
		// if anything else just run again
	}
	return 0;
}
