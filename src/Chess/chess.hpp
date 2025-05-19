#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <atomic>
#include <stack>

#ifdef __linux__
    #include "sys/socket.h"
#elif _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#endif

#include "../terminal-io/terminal.hpp"
#include "../terminal-io/colors.hpp"
#include "../client/option.hpp"
#include "../client-rand-string/random-string.hpp"
#include "../client-server-communication.hpp"
#include "../client/client-terminal-frontend/displaymanager.hpp"             

#define CONFIG_FILE_NAME "/wchesscfg"

#define CHESS_BOARD_HEIGHT 8
#define CHESS_BOARD_WIDTH 8

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
extern Options global_player_option;

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

namespace ChessEnums {

    enum class ValidateGameSquareResult {
        NO_PIECE,
        PIECE_NOT_YOURS,
        CANNOT_TAKE_OWN,
        VALID
    };

    enum class GetMoveResult {
#ifdef __linux__
        ERROR = -1,
#elif _WIN32
        ERROR_GETTING_MOVE = -1,
#endif
        QUIT,
        VALID,
        TIMER_RAN_OUT,
        UNDO,
        REDO,
        CHOOSE_MOVE_AGAIN
    };

    enum class SanitizeGetMoveResult {
        INVALID = -1,
        OPTIONS,
        VALID
    };

    enum class GameOptionResult {
        INVALID = -1,
        QUIT,
        CONTINUE,
        UNDO,
        REDO
    };

    enum class MakeMoveResult {
        KING_IN_HARM = -1,
        INVALID_MOVE,
        PIECE_TAKEN,
        PIECE_MOVED
    };
}


namespace ChessTypes {

    enum class XCoordinate { 
        A = 0, 
        B,
        C,
        D,
        E,
        F,
        G,
        H
    };

    enum class YCoordinate { 
        One = 0, 
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight
    };

    enum class GamePiece { 
        None = 0, 
        Pawn,
        Knight,
        Bishop,
        Rook,
        King,
        Queen
    };

    enum class Owner { 
        None = 0, 
        PlayerOne, 
        PlayerTwo
    };

    enum class Player { 
        PlayerOne = 1, 
        PlayerTwo
    };

    enum class GameConnectivity { 
        Local, 
        Online
    };

    enum class GetMoveType { 
        From, 
        To
    };

}

extern std::wstring enumPiece_toString(ChessTypes::GamePiece);

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
    ChessTypes::Owner mOwner;
    ChessTypes::GamePiece mPiece;

public:
    // Default constructor should initialize mPosition
    GameSquare() : mPosition({-1, -1}), mOwner(ChessTypes::Owner::None), mPiece(ChessTypes::GamePiece::None) {}

    GameSquare(ChessTypes::Owner owner, ChessTypes::GamePiece piece, Point pos)
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
                   << (mOwner == ChessTypes::Owner::None ? "None" : mOwner == ChessTypes::Owner::PlayerOne ? "Player one" : "Player two");
    }

    void setOwner(ChessTypes::Owner o) { mOwner = o; }
    ChessTypes::Owner getOwner() const { return mOwner; }

    void setPiece(ChessTypes::GamePiece p) { mPiece = p; }
    ChessTypes::GamePiece getPiece() const { return mPiece; }

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
    ChessTypes::Player mCurrentTurn;
    Point mWhitePlayerKingPos; 
    Point mBlackPlayerKingPos;
    Point mPieceCausingKingCheckPos; 

    StandardChessGameHistoryState() {}

    // Constructor without clock
    StandardChessGameHistoryState(GameSquare board[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], ChessTypes::Player turn, GameSquare* whiteKing, GameSquare* blackKing, GameSquare* checkCausingPiece) :
        mCurrentTurn(turn), mWhitePlayerKingPos(whiteKing->getPosition()), mBlackPlayerKingPos(blackKing->getPosition()) {
        if (checkCausingPiece != nullptr) 
            mPieceCausingKingCheckPos = checkCausingPiece->getPosition();
        copyStandardBoard(board, mGameBoard);
    }

    // Constructor with clock
    StandardChessGameHistoryState(GameSquare board[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], ChessTypes::Player turn, GameSquare* whiteKing, GameSquare* blackKing, GameSquare* checkCausingPiece, ChessClock clock) :
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

        std::wcout << "Current turn: " << "-- FIX ME -- " << ", White time: " << std::to_wstring(mClock.getWhiteSeconds()) << ", Black time: " << std::to_wstring(mClock.getBlackSeconds()) << std::endl; 

        mGameBoard[mWhitePlayerKingPos.m_y][mWhitePlayerKingPos.m_x].print(); std::wcout << std::endl;
        mGameBoard[mBlackPlayerKingPos.m_y][mBlackPlayerKingPos.m_x].print(); std::wcout << std::endl;
        
        if (mPieceCausingKingCheckPos.m_x != -1 && mPieceCausingKingCheckPos.m_y != 1) {
            mGameBoard[mPieceCausingKingCheckPos.m_y][mPieceCausingKingCheckPos.m_x].print(); std::wcout << std::endl;
        } else
            std::wcout << "No piece causing check (nullptr)" << std::endl;

    }
};

enum possibleMoveTypes {
    NOT_FOUND = -1, // used as a return type in readPossibleMoves
    MOVING_PIECE = 0, // piece moving from
    MOVING_TO_SQAURE, // piece / square moving to
    POSSIBLE_MOVE_OPEN_SQAURE, // possible move to open square
    POSSIBLE_MOVE_ENEMY_PIECE, // possible move to enemy piece
    POSSIBLE_MOVE_PROTECT_KING_SQUARE, // possible move to protect the king (in check) square
    POSSIBLE_MOVE_PROTECT_KING_PIECE, // possible to protect the king (in check) by taking a piece
    POSSIBLE_MOVE_SPECIAL_MOVE, // castling, enpassant
    POSSIBLE_MOVE_KING_IN_DANGER // possible move for the king to make when in danger
};
struct possibleMoveType {
    // Square Data
    GameSquare* m_boardSquare;
    // Color Data
    possibleMoveTypes possibleMoveTypeSelector;
    possibleMoveType(GameSquare* sqr, possibleMoveTypes sel)
    : m_boardSquare(sqr), possibleMoveTypeSelector(sel) {}
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

struct SquareInfo {
    ChessTypes::Owner owner;
    ChessTypes::GamePiece piece;
};

struct TakenPiece{
    std::wstring mMove;
    std::wstring mTo;
    ChessTypes::GamePiece mPiece;
    TakenPiece(){}
    TakenPiece(ChessTypes::GamePiece piece, std::wstring move, std::wstring to) : mPiece(piece), mMove(move), mTo(to) {}
};

// To be inherited by Local Game and Online Game
class StandardChessGame {
protected:
    // owner enum is used to track player turn, None will not be used, just 1 & 2
    ChessTypes::Player currentTurn = ChessTypes::Player::PlayerOne;
    bool currTurnInCheck;
    // 0 is default value, if not changed then the constructor will randomly choose someone to go first,

    SquareInfo getSquareInfo(int, int);
    uint64_t white_pawns;
    uint64_t white_knights;
    uint64_t white_bishops;
    uint64_t white_rooks;
    uint64_t white_queens;
    uint64_t white_king;
    uint64_t black_pawns;
    uint64_t black_knights;
    uint64_t black_bishops;
    uint64_t black_rooks;
    uint64_t black_queens;
    uint64_t black_king;
    uint64_t white_occupancy;
    uint64_t black_occupancy;
    uint64_t all_occupancy;


    // Moveset validation functions
    // True - Valid move
    // False - Invalid move
    bool validateMoveset(Move&);

    // True valid move
    // False invalid move
    bool verifyMove();

    // Path clearance check functions
    // True - All good
    // False - Piece in way
    bool unobstructedPathCheck();
    bool rookClearPath();
    bool bishopClearPath();

    // Pawn movement check
    // True - All good
    // False - Not good
    bool pawnMoveCheck();

    // Piece presence check
    // returns owner at point
    ChessTypes::Owner piecePresent(Point);

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
    bool kingSafeAfterMove();

    // Defense functions
    // True can defend so not checkmate
    // False cannot defend king, so checkmate
    bool canDefendKing();

    // used in only local chess game, but it is used in canDefend king so it's needed here
    bool kingCanMakeMove;

    // Determines if game is alive
    bool GameOver = false;

    // Game initialization functions
    // To be used by main thread for gameloop
    void reset();

    // 2 players, each with 16 pieces
    virtual void initGame();

    // init random turn 
    void initTurn();

    // Move conversion functions
    // generic convert
    uint64_t convertMove(std::wstring, ChessTypes::Player);

    // calls generic
    virtual int makeMove(); // basic implementation, can override tho

    // Options regarding game
    Options GameOptions;

    //! Potentially remove
    // Possible move functions
    // populates the possible move vec
    virtual bool populatePossibleMoves();

    // reads the possible moves and compares to a move
    possibleMoveTypes readPossibleMoves();

    // Board printing functions
    // print the standard board
    virtual void printBoard(ChessTypes::Player);

    // print the standard board but with moves from the movefrom
    virtual void printBoardWithMoves(ChessTypes::Player);

    // Game square validation
    // No Piece Present - 0
    // This Piece Does not belong to you - 1
    // Cannot take your own piece - 2
    // Valid - 3
    ChessEnums::ValidateGameSquareResult validateGameSquare(SquareInfo, ChessTypes::GetMoveType);

    // Vector to hold possible moves
    std::vector<possibleMoveType> possibleMoves; 
    // For move highlighting
    GameSquare* fromHighlightedPiece = nullptr;
    GameSquare* toHighlightedPiece = nullptr;

    // Not pure, bc Server Chess Std game does not need this
    ChessEnums::SanitizeGetMoveResult sanitizeGetMove(std::wstring&);

    // String to print next while loop iteration after the board is printed
    std::wstring toPrint;

    // Game connectivity
    ChessTypes::GameConnectivity GameConnectivity;

    // Game clock and input buffer
    ChessClock gameClock;
    std::wstring inputBuffer; 

    // Utility function to convert player to string
    std::wstring playerToString(ChessTypes::Player);

    // Only to be called from subclasses
    StandardChessGame(){}
    StandardChessGame(ChessTypes::GameConnectivity Connectivity) : GameConnectivity(Connectivity) {}
    StandardChessGame(ChessTypes::GameConnectivity Connectivity, ChessClock clock) : GameConnectivity(Connectivity), gameClock(clock) {}
};

// Things that do not need to be exposed for the server side of chess game logic
class ClientChessGame {
protected:
    virtual ChessEnums::GetMoveResult getMove(ChessTypes::GetMoveType) = 0;
    // Function to handle current turn chess clock
    virtual void currTurnChessClock(bool& stop, int pipe, const std::wstring& msgToOutput) = 0;
};

class StandardLocalChessGame : private StandardChessGame, public ClientChessGame{

#ifdef LEGACY_ARRAY_GAMEBOARD
    //* LEGACY
    // Used to track the game board and kings' positions
    GameSquare GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];
    GameSquare* whitePlayerKing; // These pointers need to be set
    GameSquare* blackPlayerKing;
    GameSquare* pieceCausingKingCheck = nullptr; 

    bool LpopulatePossibleMoves(GameSquare& moveFrom);
    ChessTypes::Owner LpiecePresent(Point p);
    bool LverifyMove(Move& move);
    bool LverifyMove(Move&& move);
    bool LrookClearPath(Move& move);
    bool LbishopClearPath(Move& move);
    bool LpawnMoveCheck(Move& move);
    bool LunobstructedPathCheck(Move& move);
    GameSquare* LcanDefendKing(std::vector<GameSquare*>& teamPieces);
    bool LkingSafe();
    bool LkingSafeAfterMove(GameSquare& to);
    bool LcheckMate();
    GameSquare& LconvertMove(std::wstring move, ChessTypes::Player sideToConvert);
    validateGameSquare::validateGameSquare LvalidateGameSquare(GameSquare& square, enum getMoveType getMoveType);
    possibleMoveTypes LreadPossibleMoves(GameSquare& to);
    void LstartGame();
    void LprintBoard(ChessTypes::Player playerSideToPrint);
    void LprintBoardWithMoves(ChessTypes::Player);
    GameSquare *LisolateFromInCheckMoves();
    makeMove::makeMove LmakeMove(Move&& move);
    void LinitGame();
    int LreflectAxis(int);
    
#else

public:
    // Start a standard game
    void startGame();

#endif

    // Regardless of whether or not legacy
    // Get move
    ChessEnums::GetMoveResult getMove(ChessTypes::GetMoveType) override;
    void currTurnChessClock(bool&, int, const std::wstring&) override;
    ChessEnums::GameOptionResult optionMenu(char);

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

    bool DEV_MODE_ENABLE = false;
    void DEV_MODE_PRESET();
    StandardLocalChessGame(Options, ChessClock, ChessTypes::Player, bool);
    StandardLocalChessGame(Options, ChessTypes::Player, bool);
    StandardLocalChessGame() {}


};

class StandardOnlineChessGame : public StandardChessGame, public ClientChessGame {
    
    int* pipeFds; // For use by pipe and poll for timer and getting input in non cannonical mode
    int gameSocketFd;
    ChessTypes::Player playerNum;
    std::wstring oppossingPlayerString;

    //int makeMove(Move&&) override;
    //ChessEnums::GetMoveResult getMove(ChessTypes::getMoveType) override;
    
    int preTurnCheckIn();
    bool readyForNextTurn();
    int serverSaidValidMove();
    bool nonTurnSpecificCheckIn();
    bool verifyGameServerConnection();
    bool takeMovesAndSend(std::wstring, std::wstring);
    int notTurnRecieveMove(std::wstring& move, std::wstring& moveTo);
    void currTurnChessClock(bool&, int, const std::wstring&) override;


public:

    StandardOnlineChessGame(int, ChessTypes::Player, std::wstring);
    StandardOnlineChessGame(){}
    void startGame();

    ~StandardOnlineChessGame() { delete pipeFds; }
};

// Everyone to use, idk how to sort these lmao
int char_single_digit_to_int(const char c);
std::wstring convertString(const std::string& passed);
std::string convertWString(std::wstring& passed);

