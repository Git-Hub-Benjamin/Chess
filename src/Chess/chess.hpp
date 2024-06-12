#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <atomic>
#include <stack>
#include "../terminal-io/terminal.hpp"
#include "../terminal-io/colors.hpp"
#include "../client/option.hpp"
#include "../client-rand-string/random-string.hpp"
#include "../client-server-communication.hpp"
#include "../client/client-terminal-frontend/displaymanager.hpp"

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

extern short (*pieceMovePtrs[])[2];
extern int PIECE_MOVE_COUNTS[];


struct Point{
    int m_x;
    int m_y;
    
    Point() : m_x(-1), m_y(-1) {}
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

    void print() { std::wcout << "(X: " << m_x << ", Y: " << m_y << ")"; }
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

enum GAME_CONNECTIVITY {
    LOCAL_CONNECTIVITY,
    ONLINE_CONNECTIVITY
};

extern std::wstring enumPiece_toString(GamePiece);

class ChessClock {
    int mWhitePlayerTime = -1;
    int mBlackPlayerTime = -1;

public:
    ChessClock(){}
    ChessClock(ChessClock& copy) : mWhitePlayerTime(copy.mWhitePlayerTime), mBlackPlayerTime(copy.mBlackPlayerTime) {}
    void initTime(int white, int black) {mWhitePlayerTime = white; mBlackPlayerTime = black; }

    ChessClock& operator=(const ChessClock& other) {
        if (this != &other) {
            mWhitePlayerTime = other.mWhitePlayerTime;
            mBlackPlayerTime = other.mBlackPlayerTime;
        }
        return *this;
    }

    int getWhiteSeconds() { return mWhitePlayerTime; }
    int getBlackSeconds() { return mBlackPlayerTime; }
    int* getWhiteTimeAddr() { return &mWhitePlayerTime; }
    int* getBlackTimeAddr() { return &mBlackPlayerTime; }

};

class GameSquare {
private:
    bool mfirstMoveOccurred = false; // Applies for castling, pawns, etc.
    Point mPosition;
    Owner mOwner;
    GamePiece mPiece;

public:
    // Default constructor should initialize mPosition
    GameSquare() : mPosition({-1, -1}), mOwner(NONE), mPiece(OPEN) {}

    GameSquare(Owner owner, GamePiece piece, Point pos)
        : mPosition(pos), mOwner(owner), mPiece(piece) {}

    GameSquare(const GameSquare& other)
        : mPosition(other.mPosition), mOwner(other.mOwner), mPiece(other.mPiece), mfirstMoveOccurred(other.mfirstMoveOccurred) {}

    // Corrected assignment operator
    GameSquare& operator=(const GameSquare& other) {
        if (this != &other) {
            // We can't assign to mPosition because it's const,
            // so we need to ensure that this assignment operator is only used correctly
            // where mPosition doesn't need to change.
            mOwner = other.mOwner;
            mPiece = other.mPiece;
            mPosition = other.mPosition;
            mfirstMoveOccurred = other.mfirstMoveOccurred;
        }
        return *this;
    }

    bool operator==(const GameSquare& other) const {
        return mfirstMoveOccurred == other.mfirstMoveOccurred &&
               mPosition == other.mPosition &&
               mOwner == other.mOwner &&
               mPiece == other.mPiece;
    }

    void print() const {
        std::wcout << "Pos: {" << mPosition.m_x << ", " << mPosition.m_y << "}, Piece: " 
                   << enumPiece_toString(mPiece) << ", Owner: " 
                   << (mOwner == NONE ? "None" : mOwner == PONE ? "Player one" : "Player two");
    }

    void setOwner(Owner o) { mOwner = o; }
    Owner getOwner() const { return mOwner; }

    void setPiece(GamePiece p) { mPiece = p; }
    GamePiece getPiece() const { return mPiece; }

    Point getPosition() const { return mPosition; }
    void setPosition(Point p) { mPosition = p; }

    void setFirstMoveMade() { mfirstMoveOccurred = true; }
    bool getIfFirstMoveMade() const { return mfirstMoveOccurred; }
};

extern void copyStandardBoard(GameSquare [CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], GameSquare [CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH]);

struct StandardChessGameHistoryState {
    GameSquare mGameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];
    bool usingClock = false;
    ChessClock mClock;
    Player mCurrentTurn;
    Point mWhitePlayerKingPos; 
    Point mBlackPlayerKingPos;
    Point mPieceCausingKingCheckPos; 

    StandardChessGameHistoryState() {}

    // Constructor without clock
    StandardChessGameHistoryState(GameSquare board[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], Player turn, GameSquare* whiteKing, GameSquare* blackKing, GameSquare* checkCausingPiece) :
        mCurrentTurn(turn), mWhitePlayerKingPos(whiteKing->getPosition()), mBlackPlayerKingPos(blackKing->getPosition()) {
        if (checkCausingPiece != nullptr) 
            mPieceCausingKingCheckPos = checkCausingPiece->getPosition();
        copyStandardBoard(board, mGameBoard);
    }

    // Constructor with clock
    StandardChessGameHistoryState(GameSquare board[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], Player turn, GameSquare* whiteKing, GameSquare* blackKing, GameSquare* checkCausingPiece, ChessClock clock) :
        mCurrentTurn(turn), mWhitePlayerKingPos(whiteKing->getPosition()), mBlackPlayerKingPos(blackKing->getPosition()), mClock(clock) {
        if (checkCausingPiece != nullptr) 
            mPieceCausingKingCheckPos = checkCausingPiece->getPosition();
        else
            mPieceCausingKingCheckPos = Point();
        usingClock = true;
        copyStandardBoard(board, mGameBoard);
    }

    // Copy constructor
    StandardChessGameHistoryState(const StandardChessGameHistoryState& copy) :
        mCurrentTurn(copy.mCurrentTurn), mWhitePlayerKingPos(copy.mWhitePlayerKingPos), mBlackPlayerKingPos(copy.mBlackPlayerKingPos), mPieceCausingKingCheckPos(copy.mPieceCausingKingCheckPos), usingClock(copy.usingClock) {
        if (copy.usingClock)
            mClock = copy.mClock;
        copyStandardBoard(copy.mGameBoard, mGameBoard);
    }
    
    void copyStandardBoard(const GameSquare source[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], GameSquare destination[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH]) {
        memcpy(destination, source, sizeof(GameSquare[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH]));
    }

    void print() {
        for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
            for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
                mGameBoard[row][col].print();
                std::wcout << std::endl;
            }
        }

        std::wcout << "Current turn: " << mCurrentTurn << ", White time: " << std::to_wstring(mClock.getWhiteSeconds()) << ", Black time: " << std::to_wstring(mClock.getBlackSeconds()) << std::endl; 

        mGameBoard[mWhitePlayerKingPos.m_y][mWhitePlayerKingPos.m_x].print(); std::wcout << std::endl;
        mGameBoard[mBlackPlayerKingPos.m_y][mBlackPlayerKingPos.m_x].print(); std::wcout << std::endl;
        
        if (mPieceCausingKingCheckPos.m_x != -1 && mPieceCausingKingCheckPos.m_y != 1) {
            mGameBoard[mPieceCausingKingCheckPos.m_y][mPieceCausingKingCheckPos.m_x].print(); std::wcout << std::endl;
        } else
            std::wcout << "No piece causing check (nullptr)" << std::endl;

    }
};



class Move{

    GameSquare& m_moveFrom;
    GameSquare& m_moveTo;

public:
    
    Move(GameSquare& move, GameSquare& to): m_moveFrom(move), m_moveTo(to) {}

    GameSquare& getMoveFrom() const { return m_moveFrom; }
    GameSquare& getMoveTo() const { return m_moveTo; }

    Move operator=(const Move& other) {
        if (this != &other) {
            m_moveFrom = other.m_moveFrom;
            m_moveTo = other.m_moveTo;
        }
        return *this;
    }
};

struct TakenPiece{
    std::wstring mMove;
    std::wstring mTo;
    GamePiece mPiece;
    TakenPiece(){}
    TakenPiece(GamePiece piece, std::wstring move, std::wstring to) : mPiece(piece), mMove(move), mTo(to) {}
};

// To be inherited by Local Game and Online Game
class StandardChessGame {
protected:
    // owner enum is used to track player turn, None will not be used, just 1 & 2
    Player currentTurn;

    // Used to track the game board and kings' positions
    GameSquare GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];
    GameSquare* whitePlayerKing; // These pointers need to be set
    GameSquare* blackPlayerKing;
    GameSquare* pieceCausingKingCheck = nullptr; 
    bool currTurnInCheck;

    // Moveset validation functions
    // True - Valid move
    // False - Invalid move
    bool validateMoveset(Move&);

    // True valid move
    // False invalid move
    bool verifyMove(Move&);
    bool verifyMove(Move&&);

    // Path clearance check functions
    // True - All good
    // False - Piece in way
    bool unobstructedPathCheck(Move&);
    bool rookClearPath(Move&);
    bool bishopClearPath(Move&);

    // Pawn movement check
    // True - All good
    // False - Not good
    bool pawnMoveCheck(Move&);

    // Piece presence check
    // returns owner at point
    Owner piecePresent(Point);

    // Checkmate functions
    // True, Gameover
    // False not checkmate
    bool checkMate();

    // Board boundary check
    // True on board
    // False not on board
    bool onBoard(Point&);

    // King safety checks
    // True - King is safe
    // False - King is NOT safe
    bool kingSafe();

    // True king is safe after this move is made
    // False king is not safe
    bool kingSafeAfterMove(GameSquare&);

    // Defense functions
    // True can defend so not checkmate
    // False cannot defend king, so checkmate
    GameSquare* canDefendKing(std::vector<GameSquare*>&);

    // used in only local chess game, but it is used in canDefend king so it's needed here
    bool kingCanMakeMove;

    // Determines if game is alive
    bool GameOver = false;

    // Game initialization functions
    // To be used by main thread for gameloop
    void reset();

    // 2 players, each with 16 pieces
    void initGame();

    // init random turn 
    void initTurn();

    // Move conversion functions
    // generic convert
    GameSquare& convertMove(std::wstring, Player);
    int reflectAxis(int);

    // Move execution functions
    // generic makeMove
    int makeMove(Move& move);
    // calls generic
    virtual int makeMove(Move&& move) = 0;

    // Options regarding game
    Options GameOptions;

    // Possible move functions
    // populates the possible move vec
    bool populatePossibleMoves(GameSquare&);

    // reads the possible moves and compares to a move
    bool readPossibleMoves(GameSquare&);

    // Board printing functions
    // print the standard board
    void printBoard(Player);

    // print the standard board but with moves from the movefrom
    void printBoardWithMoves(Player);

    // Game square validation
    // No Piece Present - 0
    // This Piece Does not belong to you - 1
    // Cannot take your own piece - 2
    // Valid - 3
    int validateGameSquare(GameSquare&, int);

    // Vector to hold possible moves
    std::vector<GameSquare *> possibleMoves; 

    // Virtual function to get a move, to be implemented by subclasses
    virtual int getMove(int) = 0;
    int sanitizeGetMove(std::wstring&);

    // String to print next while loop iteration after the board is printed
    std::wstring toPrint;

    // Game connectivity
    GAME_CONNECTIVITY GameConnectivity;

    // Game clock and input buffer
    ChessClock gameClock;
    std::wstring inputBuffer; 

    // Function to handle current turn chess clock
    virtual void currTurnChessClock(std::atomic_bool& stop, int pipe, const std::wstring& msgToOutput) = 0;

    // Utility function to convert player to string
    std::wstring playerToString(Player);

    // Only to be called from subclasses
    StandardChessGame(){}
    StandardChessGame(GAME_CONNECTIVITY CONNECTIVITY) : GameConnectivity(CONNECTIVITY) {}
    StandardChessGame(GAME_CONNECTIVITY CONNECTIVITY, ChessClock clock) : GameConnectivity(CONNECTIVITY), gameClock(clock) {}
};




//! Move this to a client only header since the server online chess game is in its own
class StandardLocalChessGame : public StandardChessGame {

protected:

    // overrie
    int makeMove(Move&&) override;

    // Get move
    int getMove(int) override;
    void currTurnChessClock(std::atomic_bool&, int, const std::wstring&) override;
    int optionMenu(char);

    // Will return a pointer to the one piece that you can move in check, if there is more than one then it will just return a nullptr
    GameSquare* isolateFromInCheckMoves();

    bool isLoadingState = false;
    void loadGameState(StandardChessGameHistoryState&);

    // Tracks the pieces that were taken during the game
    TakenPiece playerTakenPieces[STANDARD_CHESSGAME_PLAYER_COUNT][STANDARD_CHESSGAME_TEAM_PIECE_COUNT];
    
    
    // For game clock, Local only tho
    bool isClock = false;
;
    // For undo / redo turns
    std::stack<StandardChessGameHistoryState> undoTurn;
    std::stack<StandardChessGameHistoryState> redoTurn;


public:

    bool DEV_MODE_ENABLE = false;;
    void DEV_MODE_PRESET();
    StandardLocalChessGame(Options, ChessClock, bool);
    StandardLocalChessGame(Options, bool);
    StandardLocalChessGame() {}

    // Start a standard game
    void startGame();
};

class StandardOnlineChessGame : public StandardChessGame {
    
    int pipeFds[2]; // For use by pipe and poll for timer and getting input in non cannonical mode
    int gameSocketFd;
    Player playerNum;
    std::wstring oppossingPlayerString;

    int makeMove(Move&&) override;
    int getMove(int) override;
    
    int preTurnCheckIn();
    bool readyForNextTurn();
    int serverSaidValidMove();
    bool nonTurnSpecificCheckIn();
    bool verifyGameServerConnection();
    bool takeMovesAndSend(std::wstring, std::wstring);
    int notTurnRecieveMove(std::wstring& move, std::wstring& moveTo);
    void currTurnChessClock(std::atomic_bool&, int, const std::wstring&) override;


public:

    StandardOnlineChessGame(Options, int);
    StandardOnlineChessGame(){}
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


