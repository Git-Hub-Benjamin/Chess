//* TODO: (Highest Priority to Lowest)
// - Test windows terminal colors & add to print board functions
// - Implement bitboard
// - Implement online connectivity
// - Implement AI
//// - Fix Pawn movement

//* - Rule Todos:
// - Implement castling
// - Implement en passant
// - Add pawn promotion
// - Add draw by stalemate
// - Add draw by insufficient material
// - Add draw by threefold repetition
// - Add draw by fifty moves
// - Add draw by stalemate


//* Notes:
// - Only keep #ifdef LEGACY_ARRAY_GAMEBOARD for parts that require it for logic, otherwise it compilicates things, Ill add them back in later
// Convert all std::wcout to std::cout? Then for linux, when WChessPrint is called it will std::strings / char* to wchar_t* and std::wstrings because linux NEEDS to use std::wcout 
// for chess pieces. Chess pieces are more complicated but they should be contained to LprintBoard() and LprintBoardWithMoves()