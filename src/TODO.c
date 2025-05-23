//* TODO:
// - Implement bitboard
// - Implement online connectivity
// - Implement AI
// - Implement castling
// - Implement en passant
// - Implement stalemate
// - Test windows terminal colors & add to print board functions


// Convert all std::wcout to std::cout? Then for linux, when WChessPrint is called it will std::strings / char* to wchar_t* and std::wstrings because linux NEEDS to use std::wcout 
// for chess pieces. Chess pieces are more complicated but they should be contained to LprintBoard() and LprintBoardWithMoves()