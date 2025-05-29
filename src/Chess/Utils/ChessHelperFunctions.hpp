#include "../TypesAndEnums/ChessTypes.hpp"
#include <string>
#include <cstdint>

std::wstring convertString(const std::string& passed);
std::string convertWString(std::wstring& passed);
std::string pieceToString(ChessTypes::GamePiece piece);
int char_single_digit_to_int(const char c);
std::string toLowercase(const std::string& str);
int row_col_to_bit_index(int row, int col);
void set_bit(uint64_t& bitboard, int bit_index);
void bitprint(uint64_t);