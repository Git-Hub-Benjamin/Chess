#pragma once

#include "../TypesAndEnums/ChessTypes.hpp"
#include "SquareInfo.hpp"

struct HalfMove {
    std::string input;
    int bit_index;  // The bit index in the bitboard
    SquareInfo square;

    HalfMove(std::string input, int bit_index, SquareInfo square) : input(input), bit_index(bit_index), square(square) {}
    HalfMove(int bit_index, SquareInfo square) : bit_index(bit_index), square(square) {}
};
