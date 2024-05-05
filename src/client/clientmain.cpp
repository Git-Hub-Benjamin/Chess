#include "../chess.h"
//! Always use wcout, cout and wcout dont mix

bool running = true;
extern void local_game();
extern void online_game();

void handleOption(){
  //implement later
}

void print_wcout(wchar_t ch){
   std::wcout << ch;
}

void title_screen(){
    // menu when starting;
    std::wstring a = L"\n\n\n\n\n"
					 L"\t\t|=====================================|\n"
                     L"\t\t|                                     |\n"
                     L"\t\t|               W-CHESS               |\n"
                     L"\t\t|                                     |\n"
                     L"\t\t|           1. Local Game             |\n"
                     L"\t\t|                                     |\n"
                     L"\t\t|           2. Online Game            |\n"
                     L"\t\t|                                     |\n"
                     L"\t\t|           3. Options                |\n"
                     L"\t\t|                                     |\n"
                     L"\t\t|           4. Quit                   |\n"
                     L"\t\t|                                     |\n"
                     L"\t\t|=====================================|\n"
					 L"\n--> ";

    for(int i = 0; i < a.length(); i++){
        print_wcout(a.at(i));
    }
}

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
		break;
	}
	return -1;
}

int main()
{		
	while(running){
		title_screen();
		int opt = get_menu_option();
		if(opt == 1){
			local_game();
		}else if(opt == 2){
			online_game();
		}else if(opt ==3){

		}else if(opt == 4){
			if(Pawn1.moves == nullptr)
				uninit_moveset(); // free the heap memory in the moveset
		}
		// if anything else just run again
	}
}


/*

  a   b   c   d   e   f   g   h
+---+---+---+---+---+---+---+---+
| ♜ | ♞ | ♝ | ♛ | ♚ | ♝ | ♞ | ♜ | 8
+---+---+---+---+---+---+---+---+
| ♟ | ♟ | ♟ | ♟ | ♟ | ♟ | ♟ | ♟ | 7
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 6
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 5
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 4
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 3
+---+---+---+---+---+---+---+---+
| ♙ | ♙ | ♙ | ♙ | ♙ | ♙ | ♙ | ♙ | 2
+---+---+---+---+---+---+---+---+
| ♖ | ♘ | ♗ | ♕ | ♔ | ♗ | ♘ | ♖ | 1
+---+---+---+---+---+---+---+---+
  a   b   c   d   e   f   g   h

*/