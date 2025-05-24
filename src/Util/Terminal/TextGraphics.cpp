#include "TextGraphics.hpp"

extern Options global_player_option;



static void option_active_inactive(bool check){
	if(check)
		setTerminalColor(GREEN);
	else
		setTerminalColor(RED);
	
	std::wcout << (check == true ? "Active" : "Inactive") << std::endl;
	setTerminalColor(DEFAULT);
}


static enum WRITE_COLOR num_to_enum_write_color(int num){
	switch(num){
		case 1:
			return AQUA;
		case 2:
			return GREEN;
		case 3:
			return BLUE;
	}
	return DEFAULT; // ??
}

void color_option_active_inactive(enum WRITE_COLOR check, int playerNum){
	enum WRITE_COLOR& colorToChange = playerNum == 1 ? global_player_option.p1_color : global_player_option.p2_color;
	if(check == colorToChange){
		setTerminalColor(GREEN);
		std::wcout << "Active";
	}else{
		setTerminalColor(RED);
		std::wcout << "Inactive";
	}
	setTerminalColor(DEFAULT);
}

void piece_art_option_active_inactive(enum TEXT_PIECE_ART_COLLECTION_SELECTOR check, int playerNum){
	enum TEXT_PIECE_ART_COLLECTION_SELECTOR artToChange = playerNum == 1 ? global_player_option.whitePlayerArtSelector : global_player_option.blackPlayerArtSelector;
	if(check == artToChange){
		setTerminalColor(GREEN);
		std::wcout << "Active";
	}else{
		setTerminalColor(RED);
		std::wcout << "Inactive";
	}
	setTerminalColor(DEFAULT);
}

void eraseOrNotEraseScreen(){
	if(global_player_option.clearScreenOnPrint)
		eraseDisplay();
}

void online_menu(std::wstring plyr_id){
    eraseOrNotEraseScreen();
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|         W-CHESS - Online mode       |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    1. Random queue                  |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    2. Create private room           |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    3. Join private room             |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    X. Back - PlayerId: " << plyr_id << "   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n--> ";
}

void title_screen(){
    eraseOrNotEraseScreen();
    // menu when starting;
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           1. Local Game             |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           2. Online Game            |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           3. Options                |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           X. Quit                   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
}

void local_game_screen() {
    eraseOrNotEraseScreen();
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           1. Standard Game          |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           X. Quit                   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
}

void configure_clock_screen(){
    eraseOrNotEraseScreen();
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        1. No Chess Clock            |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        2. Chess Clock Presets       |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        3. Chess CLock Custom        |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        X. Quit                      |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
}

void clock_presets_screen(){
    eraseOrNotEraseScreen();
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|            1. 3 mins each           |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|            2. 5 mins each           |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|            3. 15 mins each          |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|            X. Quit                  |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
}

void inital_turn_screen(){
    eraseOrNotEraseScreen();
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        1. Player One first          |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        2. Player Two first          |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        3. Random                    |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|        X. Quit                      |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
}

void option_screen(){
    eraseOrNotEraseScreen();
	std::wcout << "\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
	std::wcout << "1. Change colors" << std::endl;
    std::wcout << "2. Change piece art" << std::endl;
	std::wcout << "3. Piece moveset highlighting" << "                         | ";
	option_active_inactive(global_player_option.moveHighlighting);
    std::wcout << "4. Dynamic piece moveset highlighting (CPU Intensive)" << " | ";
    option_active_inactive(global_player_option.dynamicMoveHighlighting);
	std::wcout << "5. Board history / Undo moves (Local only)" << "            | ";
	option_active_inactive(global_player_option.boardHistory);
	std::wcout << "6. Flip board on new turn     (Local only)" << "            | ";
	option_active_inactive(global_player_option.flipBoardOnNewTurn);
    std::wcout << "7. New board print clears screen" << "                      | ";
    option_active_inactive(global_player_option.clearScreenOnPrint);
	std::wcout << "8. Changes made affect W-Chess Config File            | ";
	option_active_inactive(SETTING_CHANGE_AFFECTS_CONFIG_FILE);
	std::wcout << "9. W-Chess Config File" << std::endl;
	std::wcout << "X. Back" << "\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =\n--> ";
}

void enter_private_lobby_code_menu(){
    eraseOrNotEraseScreen();
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Enter lobby code: " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 2 line
    std::wcout << L"\033[18C";  // Move cursor right 18 columns
    std::wcout.flush();
}

void displayPrivateLobbyCodeMenu(std::wstring& str){
    eraseOrNotEraseScreen();
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Lobby Code: " << str << std::endl;
    std::wcout << L"--> " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 2 line
    std::wcout << L"\033[4C";  // Move cursor right  columns
    std::wcout.flush();
}

void display_rand_queue_menu(){
    eraseOrNotEraseScreen();
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Searching for another players..." << std::endl;
    std::wcout << L"--> " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 5 line
    std::wcout << L"\033[4C";  // Move cursor right 4 columns
    std::wcout.flush();
}