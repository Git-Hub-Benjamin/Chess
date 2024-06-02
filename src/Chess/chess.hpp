#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <deque>
#include <vector>
#include <array>
#include "./terminal-io/terminal.hpp"
#include "./terminal-io/colors.hpp"
#include "./client/option.hpp"
#include "./client-rand-string/random-string.hpp"
#include "client-server-communication.hpp"

#define CONFIG_FILE_NAME "/wchesscfg"

#define CHESS_BOARD_HEIGHT 8
#define CHESS_BOARD_WIDTH 8

#define FROM_MOVE 0
#define TO_MOVE 1
#define STANDARD_CHESSGAME_PLAYER_COUNT 2
#define STANDARD_CHESSGAME_TEAM_PIECE_COUNT 16
#define STANDARD_CHESSGAME_TEAM_PIECE_ART_COUNT 7
#define STANDARD_CHESSGAME_MAXIMUM_PIECE_BOARD_COVERAGE 27

#define PAWN_POSSIBLE_MOVES 4
#define KNIGHT_POSSIBLE_MOVES 8
#define KING_POSSIBLE_MOVES 8
#define BISHOP_POSSIBLE_MOVES 28
#define ROOK_POSSIBLE_MOVES 28
#define QUEEN_POSSIBLE_MOVES 56 // ROOK + BISHOP

#define ONLINE_BUFFER_SIZE 128


struct Point{
    short m_x;
    short m_y;
    
    Point(short x, short y) : m_x(x), m_y(y) {}

    Point operator+(const Point& other) const {
        return Point(this->m_x + other.m_x, this->m_y + other.m_y); 
    }

    bool operator==(Point other){
        return this->m_x == other.m_x && this->m_y == other.m_y;
    }
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

enum Player{ // Just so we can index into things with this instead of like Owner - 1
    PlayerOne = 1,
    PlayerTwo
};


class GameSquare {

private:

    bool firstMoveOccurred = false; // Applies for castling, pawns, etc.
    const Point mPosition;
    Owner mOwner;
    GamePiece mPiece;

public:
    GameSquare(Owner owner, GamePiece piece ,Point pos): mOwner(owner), mPiece(piece), mPosition(pos) {}
    GameSquare(GameSquare* ptr): mOwner(ptr->getOwner()), mPiece(ptr->getPiece()), mPosition(ptr->getPosition()) {}

    GameSquare operator=(GameSquare* other){
        return GameSquare(other->getOwner(), other->getPiece(), other->getPosition());
    }

    GameSquare operator=(GameSquare other){
        return GameSquare(other.getOwner(), other.getPiece(), other.getPosition());
    }

    void print(){
        std::wcout << "Pos: {" << mPosition.m_x << ", " << mPosition.m_y << "}, Piece: " << enumPiece_toString(mPiece) << ", Owner: " << (mOwner == NONE ? "None" : mOwner == PONE ? "Player one" : "Player two") << std::endl;
    }

    void setOwner(Owner o) { mOwner = o; }
    Owner getOwner() const { return mOwner; }

    void setPiece(GamePiece p) { mPiece = p; }
    GamePiece getPiece() const { return mPiece; }

    Point getPosition() const { return mPosition; }

    void setFirstMoveMade() { firstMoveOccurred = true; }
    bool getIfFirstMoveMade() const { return firstMoveOccurred; }

};

std::wstring enumPiece_toString(GamePiece piece) {
    // Placeholder for the actual implementation
    switch (piece) {
        case PAWN: return L"Pawn";
        case ROOK: return L"Rook";
        case KNIGHT: return L"Knight";
        case BISHOP: return L"Bishop";
        case QUEEN: return L"Queen";
        case KING: return L"King";
        default: return L"Unknown";
    }
}

struct TakenPiece{
    std::wstring mMove;
    std::wstring mTo;
    GamePiece mPiece;
    TakenPiece(){}
    TakenPiece(GamePiece piece, std::wstring move, std::wstring to) : mPiece(piece), mMove(move), mTo(to) {}
};

struct GetMove{
    std::wstring mMove;
    int res;
    GetMove(){}
    GetMove(int res): res(res){}
    GetMove(std::wstring move, int res): mMove(move), res(res) {}
};

class Standard_ChessGame{

    // print the standard board
    void printBoard();

    // 0 Checkmate on current player
    // 1 Check on current player
    // 2 All good
    int kingSaftey();

    // 0 No piece present
    // 1 Piece does not belong to
    // 2 Cannot take own piece (After checking NOT castling)
    // 3 Valid move
    int validateMove(Move&);

    // 0 Invalid move, Moveset
    // 1 Invalid move, Piece in way
    // 2 Invalid move puts you in check
    // 3 Valid move, piece moved
    // 4 Valid move, piece taken
    int makeMove(Move&);

    // 0 Invalid move
    // 1 Piece in way
    // 2 Valid move
    int verifyMove(Move& move);

    // True - All good
    // False - Piece in way
    bool unobstructedPathCheck(Move& move);

    // True - All good
    // False - Piece in way
    bool rookClearPath(Move& move);

    // True - All good
    // False - Piece in way
    bool Standard_ChessGame::bishopClearPath(Move& move);

    // True - All good
    // False - Not good
    bool Standard_ChessGame::pawnMoveCheck(Move& move);

    bool Standard_ChessGame::checkBishopCausingCheck(std::vector<GameSquare*>& teamPieces);

    // True - Valid move
    // False - Invalid move
    bool validateMoveset(Move& move);

    // Get move
    GetMove getMove(int which);

    // Converts string move to gamesquare
    GameSquare& convertMove(std::wstring);

    // True - King is safe
    // False - King is NOT safe
    bool kingSafe();

    // returns owner at point
    enum Owner Standard_ChessGame::piecePresent(Point p);
   
    // populaes the possible move vec
    bool Standard_ChessGame::populatePossibleMoves();

    // 2 players, each with 16 pieces
    void initGame();

    std::wstring playerToString(Player);

    // Options regarding game
    Options GameOptions;

    // Tracks the pieces that were taken during the game
    TakenPiece playerTakenPieces[STANDARD_CHESSGAME_PLAYER_COUNT][STANDARD_CHESSGAME_TEAM_PIECE_COUNT];

    // Determines if game is alive
    bool GameOver;

public:
    // owner enum is used to track player turn, None will not be used, just 1 & 2
    Player currentTurn;

    GameSquare GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];
    GameSquare* KingPositions[STANDARD_CHESSGAME_PLAYER_COUNT];
    GameSquare* pieceCausingKingCheck = nullptr; 
    std::vector<GameSquare *> possibleMoves; 

    bool DEV_MODE_ENABLE = false;;
    void DEV_MODE_PRESET();
    Standard_ChessGame(bool);
    Standard_ChessGame();


    // To be used by main thread for gameloop
    void reset();

    // Start a standard game
    void startGame();

};

class Move{

    GameSquare& m_moveFrom;
    GameSquare& m_moveTo;

public:
    Move(GameSquare& move, GameSquare& to): m_moveFrom(move), m_moveTo(to) {}

    GameSquare& getMoveFrom() const { return m_moveFrom; }
    GameSquare& getMoveTo() const { return m_moveTo; }
};


// Move checking

// struct Move{
//     short x;
//     short y;
// };

struct Piece_moveset{
    struct Move* moves;
};

// Everyone to use, idk how to sort these lmao
int char_single_digit_to_int(const char c);
std::wstring convertString(const std::string& passed);
std::string convertWString(std::wstring& passed);

// Functions for client main
void local_game();

// Function for chess functionality
void handleOption();

// Functions for the client
bool kingSafe(ChessGame& game);
bool checkMate(ChessGame &game);
int makeMove(ChessGame &game, GameSqaure &from, GameSqaure &to);
bool verifyMove(ChessGame &game, GameSqaure &from, GameSqaure &to);
std::vector<GameSqaure*>* get_move_to_squares(ChessGame &game, GameSqaure& from);
void validateMovePiece(ChessGame& game, GameSqaure& movePiece, std::wstring& msg);
void validateMoveToPiece(ChessGame& game, GameSqaure& moveToSquare, std::wstring& retMsg);
bool king_safe_after_move(ChessGame& Game, GameSqaure& movePiece, GameSqaure& moveToSquare, std::wstring* toPrint = nullptr);
void print_board_with_moves(ChessGame &game, GameSqaure& from, std::vector<GameSqaure*>& vecOfSquare);

