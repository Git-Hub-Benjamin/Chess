#pragma once
#include "../chess.h"

enum WRITE_COLOR{
    DEFAULT = 0, // Default color
    RED,         // Attack
    GREEN,       // Safe king spots?
    AQUA,
    BOLD         // BOLDED WHITE
};

void erase_display(int n);
void set_terminal_color(enum WRITE_COLOR);


//! BEFORE CHANGING BOARD

// void print_board(ChessGame &game){
    
//     std::wcout << "\t\t    a   b   c   d   e   f   g   h\n" << "\t\t  +---+---+---+---+---+---+---+---+\n";
//     for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
//         std::wcout << "\t\t" << CHESS_BOARD_HEIGHT - i << " ";
//         for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

//             wchar_t piece;
//             if(game.GameBoard[i][j].ownership == NONE)
//                 piece = ' ';
//             else if(game.GameBoard[i][j].ownership == PONE)
//                 piece = piece_art_p1[game.GameBoard[i][j].piece];
//             else
//                 piece = piece_art_p2[game.GameBoard[i][j].piece];

            
//             // print section --> "| ♟ " ex.
//             std::wcout << "| " << piece << " ";
//         }
//         std::wcout << "| " << CHESS_BOARD_HEIGHT - i << std::endl;
//         std::wcout << "\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
//     }
//     std::wcout << "\t\t    a   b   c   d   e   f   g   h\n";
// }