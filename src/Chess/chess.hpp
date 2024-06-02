#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <deque>
#include <vector>
#include <array>
#include "../terminal-io/terminal.hpp"
#include "../terminal-io/colors.hpp"
#include "../client/option.hpp"
#include "../client-rand-string/random-string.hpp"
#include "../client-server-communication.hpp"

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
    int m_x;
    int m_y;
    
    Point(){}
    Point(int x, int y) : m_x(x), m_y(y) {}

    Point operator+(const Point& other) const {
        return Point(this->m_x + other.m_x, this->m_y + other.m_y); 
    }

    bool operator==(const Point other){
        return this->m_x == other.m_x && this->m_y == other.m_y;
    }

    bool operator==(const Point& other) const {
        return m_x == other.m_x && m_y == other.m_y;
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

extern std::wstring enumPiece_toString(GamePiece);


class GameSquare {

private:


    bool mfirstMoveOccurred = false; // Applies for castling, pawns, etc.
    const Point mPosition;
    Owner mOwner;
    GamePiece mPiece;

public:
    GameSquare(){}
    GameSquare(Owner owner, GamePiece piece ,Point pos): mOwner(owner), mPiece(piece), mPosition(pos) {}
    GameSquare(GameSquare* ptr): mOwner(ptr->getOwner()), mPiece(ptr->getPiece()), mPosition(ptr->getPosition()) {}

    GameSquare operator=(GameSquare* other){
        return GameSquare(other->getOwner(), other->getPiece(), other->getPosition());
    }

    GameSquare operator=(GameSquare other){
        return GameSquare(other.getOwner(), other.getPiece(), other.getPosition());
    }

    bool operator==(GameSquare other) {
        return mfirstMoveOccurred == other.getIfFirstMoveMade() && mPosition == other.getPosition() && mOwner == other.getOwner() && mPiece == other.getPiece();
    }

    void print(){
        std::wcout << "Pos: {" << mPosition.m_x << ", " << mPosition.m_y << "}, Piece: " << enumPiece_toString(mPiece) << ", Owner: " << (mOwner == NONE ? "None" : mOwner == PONE ? "Player one" : "Player two") << std::endl;
    }

    void setOwner(Owner o) { mOwner = o; }
    Owner getOwner() const { return mOwner; }

    void setPiece(GamePiece p) { mPiece = p; }
    GamePiece getPiece() const { return mPiece; }

    Point getPosition() const { return mPosition; }

    void setFirstMoveMade() { mfirstMoveOccurred = true; }
    bool getIfFirstMoveMade() const { return mfirstMoveOccurred; }

};

class Move{

    GameSquare& m_moveFrom;
    GameSquare& m_moveTo;

public:
    Move(GameSquare& move, GameSquare& to): m_moveFrom(move), m_moveTo(to) {}

    GameSquare& getMoveFrom() const { return m_moveFrom; }
    GameSquare& getMoveTo() const { return m_moveTo; }
};

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

protected:

    // print the standard board
    void printBoard();

    // print the standard board but with moves from the movefrom
    void printBoardWithMoves(GetMove);

    // True, Gameover
    // False not checkmate
    bool checkMate();

    // True, basic checks valid
    // False invalid move
    bool validateMove(Move&);

    // True piece moved
    // False piece not moved
    bool makeMove(Move&);

    // True valid move
    // False invalid move
    bool verifyMove(Move&);

    // True - All good
    // False - Piece in way
    bool unobstructedPathCheck(Move&);

    // True - All good
    // False - Piece in way
    bool rookClearPath(Move&);

    // True - All good
    // False - Piece in way
    bool bishopClearPath(Move&);

    // True can defend so not checkmate
    // False cannot defend king, so checkmate
    bool canDefendKing(std::vector<GameSquare*>&);

    // True - All good
    // False - Not good
    bool pawnMoveCheck(Move&);

    // True - Valid move
    // False - Invalid move
    bool validateMoveset(Move&);

    // Get move
    GetMove getMove(int which);

    // Converts string move to gamesquare
    GameSquare& convertMove(std::wstring);

    // True - King is safe
    // False - King is NOT safe
    bool kingSafe(GameSquare*);

    // True on board
    // False not on board
    bool onBoard(Point&);

    // returns owner at point
    Owner piecePresent(Point);
   
    // populates the possible move vec
    bool populatePossibleMoves(GameSquare&);

    // reads the possible moves and comapres to a move
    bool readPossibleMoves(GameSquare&);

    // 2 players, each with 16 pieces
    void initGame();

    std::wstring playerToString(Player);

    // Options regarding game
    Options GameOptions;

    // Tracks the pieces that were taken during the game
    TakenPiece playerTakenPieces[STANDARD_CHESSGAME_PLAYER_COUNT][STANDARD_CHESSGAME_TEAM_PIECE_COUNT];

    // Determines if game is alive
    bool GameOver;
    bool currTurnInCheck;

    // owner enum is used to track player turn, None will not be used, just 1 & 2
    Player currentTurn;

    GameSquare GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];
    GameSquare* KingPositions[STANDARD_CHESSGAME_PLAYER_COUNT];
    GameSquare* pieceCausingKingCheck = nullptr; 
    std::vector<GameSquare *> possibleMoves; 

    // to print next while loop iteration after the board is printed
    std::wstring toPrint;

public:

    bool DEV_MODE_ENABLE = false;;
    void DEV_MODE_PRESET();
    Standard_ChessGame(Options, bool);
    Standard_ChessGame(Options);
    Standard_ChessGame();


    // To be used by main thread for gameloop
    void reset();

    // Start a standard game
    void startGame();

};


// Everyone to use, idk how to sort these lmao
int char_single_digit_to_int(const char c);
std::wstring convertString(const std::string& passed);
std::string convertWString(std::wstring& passed);

// Functions for client main
void local_game();

// Function for chess functionality
void handleOption();


