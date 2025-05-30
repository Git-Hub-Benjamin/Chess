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

    enum class PossibleMovesResult {
        NotFound = -1, // used as a return type in readPossibleMoves
        MovingPiece = 0, // piece moving from
        MovingToSquare, // piece / square moving to
        PossibleMoveOpenSquare, // possible move to open square
        PossibleMoveEnemyPiece, // possible move to enemy piece
        PossibleMoveProtectKingSquare, // possible move to protect the king (in check) square
        PossibleMoveProtectKingPiece, // possible to protect the king (in check) by taking a piece
        PossibleMoveSpecialMove, // castling, enpassant
        PossibleMoveKingInDanger // possible move for the king to make when in danger
    };

    enum class MakeMoveResult {
        KingInDanger = -1,
        InvalidMove,
        PieceTaken,
        PieceMoved
    };

} // namespace ChessEnums
