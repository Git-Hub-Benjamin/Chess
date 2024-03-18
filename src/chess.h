#pragma once 
#include <cstring>
#include <string>
#include <iostream>

#define CHESS_BOARD_HEIGHT 8
#define CHESS_BOARD_WIDTH 8

#define PAWN_POSSIBLE_MOVES 4
#define KNIGHT_POSSIBLE_MOVES 8
#define KING_POSSIBLE_MOVES 8
#define BISHOP_POSSIBLE_MOVES 28
#define ROOK_POSSIBLE_MOVES 28
#define QUEEN_POSSIBLE_MOVES 64

struct Point{
    int x;
    int y;
};

enum XWIDTH{
    X0 = 0,
    X1,
    X2,
    X3,
    X4,
    X5,
    X6,
    X7
};

enum YHEIGHT{
    Y0,
    Y1,
    Y2,
    Y3,
    Y4,
    Y5,
    Y6,
    Y7
};



enum GamePiece{
    OPEN = 0,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    KING,
    QUEEN
};

enum Owner{
    NONE = 0,
    PONE,
    PTWO,
};

class GameSqaure{
public:
    struct Point pos;
    enum GamePiece piece;
    enum Owner ownership;
};


class ChessGame{

    
    void init();
    // 2 players, each with 16 pieces
    //enum GamePiece takenPieces[2][16];

public:
    // owner enum is used to track player turn, None will not be used, just 1 & 2
    enum Owner currentTurn;
    bool gameover;
    GameSqaure GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];

    ChessGame();
    void reset();


};

// Move checking

struct Move{
    short x;
    short y;
};

struct Piece_moveset{
    struct Move* moves;
};

// Everyone to use, idk how to sort these lmao
std::wstring convertString(std::string &);

// Functions for client main
void local_game();

// Function for chess functionality
void handleOption();

// Functions for the client
extern struct Piece_moveset Pawn1;
void init_moveset();
void uninit_moveset();
void print_board(ChessGame &game);
int getMove(std::wstring& dst, bool firstMove);
GameSqaure* moveConverter(ChessGame &game, std::wstring& move);
int makeMove(ChessGame &game, GameSqaure &from, GameSqaure &to);
int verifyMove(enum Owner currentTurn, GameSqaure &from, GameSqaure &to);
