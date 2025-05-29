#include <string>
#include "ChessHelperFunctions.hpp"
#include "../TypesAndEnums/ChessTypes.hpp"
#include "../../Util/Terminal/Terminal.hpp"


// Conversion from string to wstring
std::wstring convertString(const std::string& passed) {
    return std::wstring(passed.begin(), passed.end());
}

// conversion from wstring to string
std::string convertWString(std::wstring& passed){
    return std::string(passed.begin(), passed.end());
}

std::string pieceToString(ChessTypes::GamePiece piece) {
    switch (piece) {
        case ChessTypes::GamePiece::None:
            return "None";
        case ChessTypes::GamePiece::Pawn:
            return "Pawn";
        case ChessTypes::GamePiece::Knight:
            return "KNIGHT";
        case ChessTypes::GamePiece::Bishop:
            return "BISHOP";
        case ChessTypes::GamePiece::Rook:
            return "ROOK";
        case ChessTypes::GamePiece::King:
            return "KING";
        case ChessTypes::GamePiece::Queen:
            return "QUEEN";
        default:
            return "UNKNOWN";
    }
}

// Logic functions

int char_single_digit_to_int(const char c){
    return c - 48;
}


std::string toLowercase(const std::string& str) {
    std::string result;
    for (char c : str) {
        result += std::tolower(c);
    }
    return result;
}

// Helper function to get the bit index from a rank and file
// Rank 1 is row 0, File 'a' is col 0
int row_col_to_bit_index(int row, int col) {
    // Ranks are 1-8, Files are 0-7 (a=0, h=7)
    return (row - 1) * 8 + col;
}

// Function to set a bit at a given index in a bitboard
void set_bit(uint64_t& bitboard, int bit_index) {
    bitboard |= (1ULL << bit_index);
}

void bitprint(uint64_t num){
    for (int i = 0; i < 64; i++) {
        if (num & (1ULL << i)) {
            WChessPrint("1");
        } else {
            WChessPrint("0");
        }
    }
    WChessPrint("\n");
}
