#pragma once

#include <string>
#include "../TypesAndEnums/ChessTypes.hpp"

struct TakenPiece {
    std::wstring mMove;
    std::wstring mTo;
    ChessTypes::GamePiece mPiece;
    TakenPiece(){}
    TakenPiece(ChessTypes::GamePiece piece, std::wstring move, std::wstring to) : mPiece(piece), mMove(move), mTo(to) {}
};
