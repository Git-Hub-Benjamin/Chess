#include "TextPieceArt.hpp"

#ifdef _WIN32

const char* TEXT_PIECE_ART_COLLECTION[4][7] = {
    { // White pieces
        " ",
        "\xE2\x99\x9F",
        "\xE2\x99\x9E",
        "\xE2\x99\x9D",
        "\xE2\x99\x9C",
        "\xE2\x99\x9A",
        "\xE2\x99\x9B",
    },
    {
        " ",
        "\xE2\x99\x99",
        "\xE2\x99\x98",
        "\xE2\x99\x97",
        "\xE2\x99\x96",
        "\xE2\x99\x94",
        "\xE2\x99\x95",
    },
    {
        " ",
        "P",
        "N",
        "B",
        "R",
        "K",
        "Q",
    },
    {
        " ",
        "p",
        "n",
        "b",
        "r",
        "k",
        "q",
    }
};

#else
const wchar_t* TEXT_PIECE_ART_COLLECTION[PIECE_ART_COLLECTION_AMOUNT] = {
    L" ♟♞♝♜♚♛",
    L" ♙♘♗♖♔♕",
    L" PNBRKQ",
    L" pnbrkq"
};
#endif


// wchar_t piece_art_p1[7] = {' ', L'♟', L'♞', L'♝', L'♜', L'♚', L'♛'};
// wchar_t piece_art_p2[7] = {' ', L'♙', L'♘', L'♗', L'♖', L'♔', L'♕'};