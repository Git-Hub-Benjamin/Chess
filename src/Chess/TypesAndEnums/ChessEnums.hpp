#pragma once

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
        ReEnterMove,
        RePrintBoard
    };

    enum class SanitizeGetMoveResult {
        Invalid = -1,
        ReEnterMove,
        RePrintBoard,
        Options,
        Valid
    };

    enum class GameOptionResult {
        Invalid = -1,
        QUIT,
        Continue,
        UNDO,
        REDO
    };

    enum class MakeMoveResult {
        KING_IN_HARM = -1,
        INVALID_MOVE,
        PIECE_TAKEN,
        PIECE_MOVED
    };

    enum class PossibleMovesResult {
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

} // namespace ChessEnums
