#include "Chess/chess.hpp"
#include <vector>
#include <unordered_map>
#include <array>

// Conversion from string to wstring
std::wstring convertString(const std::string& passed) {
    return std::wstring(passed.begin(), passed.end());
}

// conversion from wstring to string
std::string convertWString(std::wstring& passed){
    return std::string(passed.begin(), passed.end());
}

// Logic functions

std::wstring enumPiece_toString(GamePiece piece) {
    // Placeholder for the actual implementation
    switch (piece) {
        case PAWN: return L"Pawn";
        case ROOK: return L"Rook";
        case KNIGHT: return L"Knight";
        case BISHOP: return L"Bishop";
        case QUEEN: return L"Queen";
        case KING: return L"King";
        default: return L"Unknown";
    }
}

int char_single_digit_to_int(const char c){
    return c - 48;
}