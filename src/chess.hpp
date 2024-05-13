#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <deque>
#include <vector>
#include "./terminal-io/terminal.hpp"
#include "./terminal-io/colors.hpp"

#define CHESS_BOARD_HEIGHT 8
#define CHESS_BOARD_WIDTH 8

#define PAWN_POSSIBLE_MOVES 4
#define KNIGHT_POSSIBLE_MOVES 8
#define KING_POSSIBLE_MOVES 8
#define BISHOP_POSSIBLE_MOVES 28
#define ROOK_POSSIBLE_MOVES 28
#define QUEEN_POSSIBLE_MOVES 56 // ROOK + BISHOP


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

std::wstring enumPiece_toString(enum GamePiece p);

class GameSqaure{
public:
    GameSqaure(){}
    GameSqaure(const GameSqaure& copy): ownership(copy.ownership), piece(copy.piece), pos(copy.pos){}
    struct Point pos;
    enum GamePiece piece;
    enum Owner ownership;

    void print(){
        std::wcout << "Pos: {" << pos.x << ", " << pos.y << "}, Piece: " << enumPiece_toString(piece) << ", Owner: " << (ownership == NONE ? "None" : ownership == PONE ? "Player one" : "Player two") << std::endl;
    }
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
    GameSqaure* KingPositions[2]; // 2 because there are 2 players, update king position in makeMove function whenever king moves (NOTE CASTLING TOO)
    GameSqaure& pieceCausingKingCheck = GameBoard[0][0]; 
    // i dont like doing this, but idk what else to do to get rid of the warning / error,
    // either way this will be updated whenever a piece causes a check on a king

    bool DEV_MODE_ENABLE;
    void DEV_MODE_PRESET();
    ChessGame(bool);
    void reset();

    // Options regarding game
    bool moveHighlighting = false;
    enum WRITE_COLOR p1_color;// BOLD
    enum WRITE_COLOR p2_color;// BOLD

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
std::string convertString(std::string &);

// Functions for client main
void local_game();

// Function for chess functionality
void handleOption();

// Functions for the client
extern struct Piece_moveset Pawn1;
void init_moveset();
void uninit_moveset();
void print_messages(std::vector<std::wstring>&);
void print_board(ChessGame &game);
bool kingSafe(ChessGame& game);
bool checkMate(ChessGame &game);
int getMove(std::wstring& dst);
GameSqaure* moveConverter(ChessGame &game, std::wstring& move);
int makeMove(ChessGame &game, GameSqaure &from, GameSqaure &to);
bool verifyMove(ChessGame &game, GameSqaure &from, GameSqaure &to);
std::vector<GameSqaure*>* get_move_to_squares(ChessGame &game, GameSqaure& from);
void validateMovePiece(ChessGame& game, GameSqaure& movePiece, std::wstring& msg);
void validateMoveToPiece(ChessGame& game, GameSqaure& moveToSquare, std::wstring& retMsg);
void print_board_with_moves(ChessGame &game, GameSqaure& from, std::vector<GameSqaure*>& vecOfSquare);

