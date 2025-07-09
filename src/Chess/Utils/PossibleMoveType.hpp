#pragma once

#include "../TypesAndEnums/ChessEnums.hpp"
#include "../LegacyArray/GameSquare.hpp"

//! I have no idea why m_boardSquare is needed
struct PossibleMoveType {
    // Square Data < -- LEGACY
    GameSquare* m_boardSquare;
    int squareIndex;
    // Color Data
    ChessEnums::PossibleMovesResult PossibleMoveTypeSelector;

    PossibleMoveType(GameSquare* sqr, ChessEnums::PossibleMovesResult sel) // < -- LEGACY
    : m_boardSquare(sqr), PossibleMoveTypeSelector(sel) {}
    PossibleMoveType(int sqr, ChessEnums::PossibleMovesResult sel)
    : squareIndex(sqr), PossibleMoveTypeSelector(sel) {}
};