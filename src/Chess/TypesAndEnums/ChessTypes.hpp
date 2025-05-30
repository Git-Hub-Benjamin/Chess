#pragma once

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
        Eight = 0,
        Seven,
        Six,
        Five,
        Four,
        Three,
        Two,
        One  
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
        To,
        Promotion
    };

    enum class CastlingType {
        KingSide,
        QueenSide
    };

} // namespace ChessTypes