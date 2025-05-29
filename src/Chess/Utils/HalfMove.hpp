#pragma once

#include "../TypesAndEnums/ChessTypes.hpp"
#include "SquareInfo.hpp"

struct HalfMove {
    int index;
    SquareInfo square;

    HalfMove(int index, SquareInfo square) : index(index), square(square) {}
};
