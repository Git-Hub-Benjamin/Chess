#include "../chess.hpp"
//! Always use wcout, wcout and wcout dont mix

bool running = true;
extern void local_game(bool);
extern void online_game();

// GLOBAL
enum WRITE_COLOR playerOneColor;
enum WRITE_COLOR playerTwoColor;

void handleOption(){
  //implement later
}

void print_wcout(wchar_t ch){
   std::wcout << ch;
}

void option_sreen(){
	std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           1. Player One color       |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           2. Player Two color       |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           3. Undo move (local)      |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|           4. Back                   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
	std::wcout << "\n--> ";
}

void color_select_screen(){
	std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               W-CHESS               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               1. AQUA       	       |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               2. GREEN       	   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               3. BLUE			   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|               4. Back               |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n" << std::endl;
	std::wcout << "\n--> ";
}

enum WRITE_COLOR num_to_enum_write_color(int num){
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
	std::wcout << "\n--> ";
}

//* For future use
// std::string a = "\n\n\n\n\n" 
// 				 "\t\t\t\t|=====================================|\n"
// 				 "\t\t\t\t|                                     |\n" 
// 				 "\t\t\t\t|               W-CHESS               |\n" 
// 				 "\t\t\t\t| 		 							 |\n" 
// 				 "\t\t\t\t|           1. Local Game 			 |\n" 
// 				 "\t\t\t\t| 									 |\n" 
// 				 "\t\t\t\t|           2. Online Game            |\n" 
// 				 "\t\t\t\t| 									 |\n" 
// 				 "\t\t\t\t|           3. Options  				 |\n" 
// 				 "\t\t\t\t| 									 |\n" 
// 				 "\t\t\t\t|           4. Quit 				     |\n" 
// 				 "\t\t\t\t| 									 |\n" 
// 				 "\t\t\t\t|=====================================|\n" 
// 				 "\n--> "; 

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
		if(str[0] == L'5' || str[0] == L'D' || str[0] == L'd')
			return 5;
		break;
	}
	return -1;
}

int main()
{		
	// Set the global locale to support UTF-8 encoding
    std::locale::global(std::locale("en_US.UTF-8"));
	set_terminal_color(DEFAULT);
	std::wcout << "\n\n\n\n\n" << std::endl;

	// To make chess pieces able to print
	while(running){
		title_screen();
		int opt = get_menu_option();
		if(opt == 1 || opt == 5){
			local_game(opt == 5 ? true : false);
		}else if(opt == 2){
			online_game();
		}else if(opt ==3){
			option_sreen();
			int option_opt = get_menu_option();
			if(option_opt == 1 || option_opt == 2){
				color_select_screen();
				int color_opt = get_menu_option();
				if(option_opt == 1){
					playerOneColor = num_to_enum_write_color(color_opt);
				}else{
					playerTwoColor = num_to_enum_write_color(color_opt);
				}

			}else if(opt == 3)
				std::wcout << "Not implemented, try again later." << std::endl;
			else
				std::wcout << "Back." << std::endl;
				
		}else if(opt == 4){
			if(Pawn1.moves == nullptr)
				uninit_moveset(); // free the heap memory in the moveset
			running = false;
			std::wcout << "Have a good day..." << std::endl;
		}
		// if anything else just run again
	}
}


