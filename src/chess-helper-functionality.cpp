#include "Chess/chess.hpp"

// Conversion from string to wstring
std::wstring convertString(const std::string& passed) {
    return std::wstring(passed.begin(), passed.end());
}

// conversion from wstring to string
std::string convertWString(std::wstring& passed){
    return std::string(passed.begin(), passed.end());
}

std::wstring enumPiece_toString(ChessTypes::GamePiece piece) {
    switch (piece) {
        case ChessTypes::GamePiece::None:
            return L"None";
        case ChessTypes::GamePiece::Pawn:
            return L"Pawn";
        case ChessTypes::GamePiece::Knight:
            return L"KNIGHT";
        case ChessTypes::GamePiece::Bishop:
            return L"BISHOP";
        case ChessTypes::GamePiece::Rook:
            return L"ROOK";
        case ChessTypes::GamePiece::King:
            return L"KING";
        case ChessTypes::GamePiece::Queen:
            return L"QUEEN";
        default:
            return L"UNKNOWN";
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
int rank_file_to_bit_index(int rank, int file) {
    // Ranks are 1-8, Files are 0-7 (a=0, h=7)
    return (rank - 1) * 8 + file;
}

// Function to set a bit at a given index in a bitboard
void set_bit(uint64_t& bitboard, int bit_index) {
    bitboard |= (1ULL << bit_index);
}

