#include <iostream>
#include <cstdint>
#include <string.h>
#include <cwctype>
#include <windows.h>
#include "src/Chess/TypesAndEnums/ChessTypes.hpp"
#include "src/Chess/Utils/ChessConstants.hpp"
#include "src/Chess/TypesAndEnums/ChessEnums.hpp"
#include "src/Util/Terminal/Terminal.hpp"
#include "src/Chess/LegacyArray/GameSquare.hpp"
#include "src/Util/Terminal/TextPieceArt.hpp"
#include "src/Chess/Utils/ChessConstants.hpp"

int main() {

    for (int i = 0; i < 64; i++) {
        std::cout << row_col_to_bit_index(i / 8, i % 8) << std::endl;
    }

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            std::cout << row_col_to_bit_index(row, col) << std::endl;
        }
    }
    return 0;
}
