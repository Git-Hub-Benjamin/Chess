#pragma once

#include "../TypesAndEnums/ChessEnums.hpp"
#include "GameSquare.hpp"

struct possibleMoveType {
    // Square Data
    GameSquare* m_boardSquare;
    // Color Data
    ChessEnums::PossibleMovesResult possibleMoveTypeSelector;
    possibleMoveType(GameSquare* sqr, ChessEnums::PossibleMovesResult sel)
    : m_boardSquare(sqr), possibleMoveTypeSelector(sel) {}
};