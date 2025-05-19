#include "../TypesAndEnums/ChessTypes.hpp"
#include <string>
#include <cstdint>

std::wstring convertString(const std::string& passed);
std::string convertWString(std::wstring& passed);
std::wstring enumPiece_toString(ChessTypes::GamePiece piece);
int char_single_digit_to_int(const char c);
std::string toLowercase(const std::string& str);
int rank_file_to_bit_index(int rank, int file);
void set_bit(uint64_t& bitboard, int bit_index);