#pragma once

static short PawnUp[PAWN_POSSIBLE_MOVES][2] = { {0, -1}, {1, -1}, {-1, -1}, {0, -2} };
static short PawnDown[PAWN_POSSIBLE_MOVES][2] = { {0, 1},  {-1, 1}, {1, 1}  , {0, 2}  };
static short KnightMoves[KNIGHT_POSSIBLE_MOVES][2] = { {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2} };
static short KingMoves[KING_POSSIBLE_MOVES][2] = {
    {0, 1}, {1, 1}, {1, 0}, {1, -1}, 
    {0, -1}, {-1, -1}, {-1, 0}, {-1, 1},
    {2, 0}, {-2, 0} // Castling
};
static short BishopMoves[BISHOP_POSSIBLE_MOVES][2] = {
    {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},     // bottom left to top right
    {-1, -1}, {-2, -2}, {-3, -3}, {-4, -4}, {-5, -5}, {-6, -6}, {-7, -7},  // top right to bottom left
    {-1, 1}, {-2, 2}, {-3, 3}, {-4, 4}, {-5, 5}, {-6, 6}, {-7, 7}, // bottom right to top left
    {1, -1}, {2, -2}, {3, -3}, {4, -4}, {5, -5}, {6, -6}, {7, -7}  // top left to bottom right
};
static short RookMoves[ROOK_POSSIBLE_MOVES][2] = {
    {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},       // bottom left to bottom right
    {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},       // bottom left to top left
    {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0}, {-5, 0}, {-6, 0}, {-7, 0}, // top right to top left
    {0, -1}, {0, -2}, {0, -3}, {0, -4}, {0, -5}, {0, -6}, {0, -7}  // top right to bottom right
};
static short QueenMoves[QUEEN_POSSIBLE_MOVES][2] = {
    {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},       // Diagonal: bottom left to top right
    {-1, -1}, {-2, -2}, {-3, -3}, {-4, -4}, {-5, -5}, {-6, -6}, {-7, -7}, // Diagonal: top right to bottom left
    {-1, 1}, {-2, 2}, {-3, 3}, {-4, 4}, {-5, 5}, {-6, 6}, {-7, 7}, // Diagonal: bottom right to top left
    {1, -1}, {2, -2}, {3, -3}, {4, -4}, {5, -5}, {6, -6}, {7, -7}, // Diagonal: top left to bottom right
    {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},       // Horizontal: bottom left to bottom right
    {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},       // Vertical: bottom left to top left
    {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0}, {-5, 0}, {-6, 0}, {-7, 0}, // Horizontal: top right to top left
    {0, -1}, {0, -2}, {0, -3}, {0, -4}, {0, -5}, {0, -6}, {0, -7}  // Vertical: top right to bottom right
};

short (*pieceMovePtrs[])[2] = {
    PawnDown,
    PawnUp,
    KnightMoves,
    BishopMoves,
    RookMoves,
    KingMoves,
    QueenMoves
};

int PIECE_MOVE_COUNTS[] = {PAWN_POSSIBLE_MOVES, KNIGHT_POSSIBLE_MOVES, BISHOP_POSSIBLE_MOVES, ROOK_POSSIBLE_MOVES, KING_POSSIBLE_MOVES, QUEEN_POSSIBLE_MOVES};