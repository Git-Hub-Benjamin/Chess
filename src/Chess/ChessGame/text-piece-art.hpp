#pragma once
#include "../../terminal-io/colors.hpp"

#define PIECE_ART_COLLECTION_AMOUNT 4
#define GAME_COLORS 8

enum TEXT_PIECE_ART_COLLECTION_SELECTOR{
    STD_PIECE_ART_P1 = 0,
    STD_PIECE_ART_P2,
    STD_PIECE_CHAR_P1,
    STD_PIECE_CHAR_P2,
};

extern const wchar_t* TEXT_PIECE_ART_COLLECTION[PIECE_ART_COLLECTION_AMOUNT];
