#pragma once

#include "ChessConstants.hpp"

extern short PawnUp[PAWN_POSSIBLE_MOVES][2];
extern short PawnDown[PAWN_POSSIBLE_MOVES][2];
extern short KnightMoves[KNIGHT_POSSIBLE_MOVES][2];
extern short KingMoves[KING_POSSIBLE_MOVES][2];
extern short BishopMoves[BISHOP_POSSIBLE_MOVES][2];
extern short RookMoves[ROOK_POSSIBLE_MOVES][2];
extern short QueenMoves[QUEEN_POSSIBLE_MOVES][2];

extern short (*PieceMovePtrs[])[2];

extern int PieceMoveCounts[];