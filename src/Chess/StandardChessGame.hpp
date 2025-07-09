#pragma once

#include <cstdint>

#include "TypesAndEnums/ChessEnums.hpp"
#include "TypesAndEnums/ChessTypes.hpp"
#include "Utils/SquareInfo.hpp"
#include "Utils/Point.hpp"
#include "Utils/ChessClock.hpp"
#include "../Client/Options/Options.hpp"
#include "Utils/Move.hpp"
#include "Utils/PossibleMoveType.hpp"

// To be inherited by Local Game and Online Game
class StandardChessGame
{
protected:
    // owner enum is used to track player turn, None will not be used, just 1 & 2
    ChessTypes::Player currentTurn = ChessTypes::Player::PlayerOne;
    bool currTurnInCheck;
    // 0 is default value, if not changed then the constructor will randomly choose someone to go first,

    SquareInfo getSquareInfo(int);
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

    // Test if the king is safe after a move is made
    // Returns:
    // - True - King is safe
    // - False - King is not safe
    bool testKingSafe(Move& move);

    // Moveset validation functions
    // True - Valid move
    // False - Invalid move
    bool validateMoveset(Move& move);

    // Verifies the move via moveset, path clearance check, and king safety check
    // Returns:
    // - True - Valid move
    // - False - Invalid move
    bool verifyMove(Move& move);

    // Path clearance check functions
    // Returns:
    // - True - All good
    // - False - Piece in way
    bool unobstructedPathCheck(Move& move);

    // Returns:
    // - True - All good
    // - False - Piece in way
    bool rookClearPath(Move& move);

    // Returns:
    // - True - All good
    // - False - Piece in way
    bool bishopClearPath(Move& move);

    // Pawn movement check
    // True - All good
    // False - Not good
    bool pawnMoveCheck(Move& move);

    // Print bitboards
    // Returns:
    // - None
    void printBitboards();

    // Move piece
    // Returns:
    // - PieceMoved: Piece moved successfully
    // - PieceTaken: Piece taken successfully
    // - InvalidMove: Invalid move
    // - KingInDanger: King is in danger
    // - PawnPromotion: Pawn promotion
    ChessEnums::MakeMoveResult movePiece(Move&);

    // isPawnPromotion
    // Returns:
    // - True - Pawn promotion
    // - False - Not pawn promotion
    bool isPawnPromotion(Move&);

    // Piece presence check
    // Returns:
    // - Owner of piece at point
    ChessTypes::Owner piecePresent(Point);

    // Checkmate functions
    // Returns:
    // - True - Checkmate
    // - False - Not checkmate
    bool checkMate();

    // Board boundary check
    // Returns:
    // - True - On board
    // - False - Not on board
    bool onBoard(Point& p);

    // King safety check
    // Returns:
    // - True - King is safe
    // - False - King is NOT safe
    bool kingSafe();

    // True king is safe after this move is made
    // False king is not safe
    bool kingSafeAfterMove();

    // canDefendKing
    // Returns:
    // - True - Can defend so not checkmate
    // - False - Cannot defend king, so checkmate
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
    HalfMove convertMove(std::string);

    // calls generic
    virtual ChessEnums::MakeMoveResult makeMove(Move& move); // basic implementation, can override tho

    // Options regarding game
    Options GameOptions;

    // Populates the possible moves vector
    // Returns:
    // - True - There is at least one move
    // - False - No moves from this piece
    virtual bool populatePossibleMoves(HalfMove& fromSquare);

    std::vector<int> possibleMoves;

    bool readPossibleMoves(int);

    // Board printing functions
    // print the standard board
    virtual void printBoard();

    // print the standard board but with moves from the movefrom
    virtual void printBoardWithMoves();

    // Validates the game square
    // Returns:
    // - No Piece Present - 0
    // - This Piece Does not belong to you - 1
    // - Cannot take your own piece - 2
    // - Valid - 3
    ChessEnums::ValidateGameSquareResult validateGameSquare(SquareInfo, ChessTypes::GetMoveType);

    // Not pure, bc Server Chess Std game does not need this
    ChessEnums::SanitizeGetMoveResult sanitizeGetMove(std::string &);

    // String to print next while loop iteration after the board is printed
    std::string toPrint;

    // Game connectivity
    ChessTypes::GameConnectivity GameConnectivity;

    // Game clock and input buffer
    ChessClock gameClock;
    std::string inputBuffer;

    // Utility function to convert player to string
    std::string playerToString(ChessTypes::Player);

    // Only to be called from subclasses
    StandardChessGame() {}
    StandardChessGame(ChessTypes::GameConnectivity Connectivity) : GameConnectivity(Connectivity) {}
    StandardChessGame(ChessTypes::GameConnectivity Connectivity, ChessClock clock) : GameConnectivity(Connectivity), gameClock(clock) {}
};