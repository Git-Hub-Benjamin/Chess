#include "textgraphic.hpp"

static void option_active_inactive(bool check){
	if(check)
		set_terminal_color(GREEN);
	else
		set_terminal_color(RED);
	
	std::wcout << (check == true ? "Active" : "Inactive") << std::endl;
	set_terminal_color(DEFAULT);
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
		set_terminal_color(GREEN);
		std::wcout << "Active";
	}else{
		set_terminal_color(RED);
		std::wcout << "Inactive";
	}
	set_terminal_color(DEFAULT);

}

void online_menu(std::wstring plyr_id){
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
    std::wcout << "\t\t\t|    4. Back - PlayerId: " << plyr_id << "   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n--> ";
}

void title_screen(){
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
    std::wcout << "\t\t\t|           4. Quit                   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
}

void option_screen(){
	std::wcout << "\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =\n";
	std::wcout << "1. Change colors" << std::endl;
	std::wcout << "2. Piece moveset highlighting (Local only)" << " | ";
	option_active_inactive(global_player_option.moveHighlighting);
	std::wcout << "3. Board history / Undo moves (Local only)" << " | ";
	option_active_inactive(global_player_option.boardHistory);
	std::wcout << "4. Flip board on new turn     (Local only)" << " | ";
	option_active_inactive(global_player_option.flipBoardOnNewTurn);
	std::wcout << "5. Changes made affect W-Chess Config File | ";
	option_active_inactive(SETTING_CHANGE_AFFECTS_CONFIG_FILE);
	std::wcout << "6. W-Chess Config File" << std::endl;
	std::wcout << "7. Back" << "\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =\n--> ";
}

void enter_private_lobby_code_menu(){
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Enter lobby code: " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 2 line
    std::wcout << L"\033[18C";  // Move cursor right 18 columns
    std::wcout.flush();
}

void display_private_lobby_code_menu(std::wstring& str){
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
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Searching for another players..." << std::endl;
    std::wcout << L"--> " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 5 line
    std::wcout << L"\033[4C";  // Move cursor right 4 columns
    std::wcout.flush();
}