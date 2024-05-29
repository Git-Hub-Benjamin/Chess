#include "textgraphic.hpp"

void online_menu(std::wstring& plyr_id){
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