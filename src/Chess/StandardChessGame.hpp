#pragma once

#include <cstdint>

#include "TypesAndEnums/ChessEnums.hpp"
#include "TypesAndEnums/ChessTypes.hpp"
#include "Utils/SquareInfo.hpp"
#include "Utils/Point.hpp"
#include "Utils/ChessClock.hpp"
#include "../Client/Options/Options.hpp"

// To be inherited by Local Game and Online Game
class StandardChessGame
{
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
    // bool validateMoveset(Move&);

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
    bool onBoard(Point &);

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
    // ChessEnums::PossibleMovesResult readPossibleMoves();

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

    // Not pure, bc Server Chess Std game does not need this
    ChessEnums::SanitizeGetMoveResult sanitizeGetMove(std::wstring &);

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
    StandardChessGame() {}
    StandardChessGame(ChessTypes::GameConnectivity Connectivity) : GameConnectivity(Connectivity) {}
    StandardChessGame(ChessTypes::GameConnectivity Connectivity, ChessClock clock) : GameConnectivity(Connectivity), gameClock(clock) {}
};