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

std::wstring enumPiece_toString(GamePiece piece) {
    switch (piece) {
        case OPEN:
            return L"OPEN";
        case PAWN:
            return L"PAWN";
        case KNIGHT:
            return L"KNIGHT";
        case BISHOP:
            return L"BISHOP";
        case ROOK:
            return L"ROOK";
        case KING:
            return L"KING";
        case QUEEN:
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
