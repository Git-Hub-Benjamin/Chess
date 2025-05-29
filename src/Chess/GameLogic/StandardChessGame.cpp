#include <random>
#include <cwctype>

#include "../../Util/Terminal/Terminal.hpp"
#include "../TypesAndEnums/ChessTypes.hpp"
#include "../TypesAndEnums/ChessEnums.hpp"
#include "../StandardChessGame.hpp"
#include "../Utils/ChessConstants.hpp"
#include "../../Util/Terminal/TextPieceArt.hpp"
#include "../Utils/ChessHelperFunctions.hpp"
#include "../Utils/ChessConstants.hpp"
#include "../Utils/Move.hpp"

SquareInfo StandardChessGame::getSquareInfo(int bit_index) {
    SquareInfo info = {ChessTypes::Owner::None, ChessTypes::GamePiece::None}; // Default to empty

    if ((white_occupancy >> bit_index) & 1) {
        info.owner = ChessTypes::Owner::PlayerOne;
        if ((white_pawns >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Pawn;
        else if ((white_knights >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Knight;
        else if ((white_bishops >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Bishop;
        else if ((white_rooks >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Rook;
        else if ((white_queens >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Queen;
        else if ((white_king >> bit_index) & 1) info.piece = ChessTypes::GamePiece::King;
    } else if ((black_occupancy >> bit_index) & 1) {
        info.owner = ChessTypes::Owner::PlayerTwo;
        if ((black_pawns >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Pawn;
        else if ((black_knights >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Knight;
        else if ((black_bishops >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Bishop;
        else if ((black_rooks >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Rook;
        else if ((black_queens >> bit_index) & 1) info.piece = ChessTypes::GamePiece::Queen;
        else if ((black_king >> bit_index) & 1) info.piece = ChessTypes::GamePiece::King;
    }

    return info;
}

void StandardChessGame::reset() {
    GameOver = false;
    initTurn();
    initGame();
}

void StandardChessGame::initTurn() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, 1);
    currentTurn = (static_cast<ChessTypes::Player>(distribution(gen) + 1));
}

// Helper functions for bitboard

int row_col_to_bit_index(int, int);
void set_bit(uint64_t&, int);

void StandardChessGame::initGame(){
    white_pawns = 0; white_knights = 0; white_bishops = 0;
    white_rooks = 0; white_queens = 0; white_king = 0;
    black_pawns = 0; black_knights = 0; black_bishops = 0;
    black_rooks = 0; black_queens = 0; black_king = 0;
    white_occupancy = 0; black_occupancy = 0; all_occupancy = 0;

    // 2. Set bits for White pieces

    // White Pawns: Rank 2 (row 1)
    for (int file = 0; file < 8; ++file) {
        set_bit(white_pawns, row_col_to_bit_index(7, file));
    }

    // White Rooks: a1 and h1 (Rank 1, files 0 and 7)
    set_bit(white_rooks, row_col_to_bit_index(8, 0));
    set_bit(white_rooks, row_col_to_bit_index(8, 7));

    // White Knights: b1 and g1 (Rank 1, files 1 and 6)
    set_bit(white_knights, row_col_to_bit_index(8, 1));
    set_bit(white_knights, row_col_to_bit_index(8, 6));

    // White Bishops: c1 and f1 (Rank 1, files 2 and 5)
    set_bit(white_bishops, row_col_to_bit_index(8, 2));
    set_bit(white_bishops, row_col_to_bit_index(8, 5));

    // White Queen: d1 (Rank 1, file 3)
    set_bit(white_queens, row_col_to_bit_index(8, 3));

    // White King: e1 (Rank 1, file 4)
    set_bit(white_king, row_col_to_bit_index(8, 4));

    // 3. Set bits for Black pieces

    // Black Pawns: Rank 7 (row 6)
    for (int file = 0; file < 8; ++file) {
        set_bit(black_pawns, row_col_to_bit_index(2, file));
    }

    // Black Rooks: a8 and h8 (Rank 8, files 0 and 7)
    set_bit(black_rooks, row_col_to_bit_index(1, 0));
    set_bit(black_rooks, row_col_to_bit_index(1, 7));

    // Black Knights: b8 and g8 (Rank 8, files 1 and 6)
    set_bit(black_knights, row_col_to_bit_index(1, 1));
    set_bit(black_knights, row_col_to_bit_index(1, 6));

    // Black Bishops: c8 and f8 (Rank 8, files 2 and 5)
    set_bit(black_bishops, row_col_to_bit_index(1, 2));
    set_bit(black_bishops, row_col_to_bit_index(1, 5));

    // Black Queen: d8 (Rank 8, file 3)
    set_bit(black_queens, row_col_to_bit_index(1, 3));

    // Black King: e8 (Rank 8, file 4)
    set_bit(black_king, row_col_to_bit_index(1, 4));

    // 4. Calculate occupancy bitboards
    white_occupancy = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_king;
    black_occupancy = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_king;
    all_occupancy = white_occupancy | black_occupancy;

    WChessPrint("White Occupancy: ");
    bitprint(white_occupancy);
    WChessPrint("White Pawns: ");
    bitprint(white_pawns);
    WChessPrint("White Knights: ");
    bitprint(white_knights);
    WChessPrint("White Bishops: ");
    bitprint(white_bishops);
    WChessPrint("White Rooks: ");
    bitprint(white_rooks);
    WChessPrint("White Queens: ");
    bitprint(white_queens);
    WChessPrint("White King: ");
    bitprint(white_king);

    WChessPrint("Black Occupancy: ");
    bitprint(black_occupancy);
    WChessPrint("Black Pawns: ");
    bitprint(black_pawns);
    WChessPrint("Black Knights: ");
    bitprint(black_knights);
    WChessPrint("Black Bishops: ");
    bitprint(black_bishops);
    WChessPrint("Black Rooks: ");
    bitprint(black_rooks);
    WChessPrint("Black Queens: ");
    bitprint(black_queens);
    WChessPrint("Black King: ");
    bitprint(black_king);

    WChessPrint("All Occupancy: ");
    bitprint(all_occupancy);

}

// True - There is at least one move
// False - No moves from this piece
bool StandardChessGame::populatePossibleMoves() { return false; }

// True found matching move with possibleMoves
// False not found
//ChessEnums::PossibleMovesResult StandardChessGame::readPossibleMoves() { return ChessEnums::PossibleMovesResult::NOT_FOUND; }

// No Piece Present - 0
// This Piece Does not belong to you - 1
// Cannot take your own piece - 2
// Valid - 3
ChessEnums::ValidateGameSquareResult StandardChessGame::validateGameSquare(SquareInfo square, ChessTypes::GetMoveType getMoveType){
    if (getMoveType == ChessTypes::GetMoveType::From) {

        if(square.owner == ChessTypes::Owner::None)
            return ChessEnums::ValidateGameSquareResult::NO_PIECE;

        if(static_cast<ChessTypes::Player>(square.owner) != currentTurn)
            return ChessEnums::ValidateGameSquareResult::PIECE_NOT_YOURS;

    } else 
        if(static_cast<ChessTypes::Player>(square.owner) == currentTurn)
            return ChessEnums::ValidateGameSquareResult::CANNOT_TAKE_OWN;
    
    return ChessEnums::ValidateGameSquareResult::VALID;
}

//! This should be StandardLocalChessGame::printBoard() but its giving me an error
void StandardChessGame::printBoard() {
    if (GameOptions.clearScreenOnPrint)
        eraseDisplay();
    setTerminalColor(DEFAULT);

#ifdef _WIN32
    WChessPrint("\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n");
    WChessPrint("\t\t\t  +---+---+---+---+---+---+---+---+\n");
#else
    WChessPrint(L"\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n");
    WChessPrint(L"\t\t\t  +---+---+---+---+---+---+---+---+\n");
#endif

    for (int row = 0; row < CHESS_BOARD_HEIGHT; ++row) {
        // Determine the rank label for the current row
        int rank_label = (GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo)
                             ? row + 1 // For flipped view, rank 1 is at the top
                             : CHESS_BOARD_HEIGHT - row; // Standard view, rank 8 is at the top

#ifdef _WIN32
        WChessPrint("\t\t\t"); WChessPrint(std::to_string(rank_label).c_str()); WChessPrint(" ");
#else
        WChessPrint(L"\t\t\t"); WChessPrint(std::to_wstring(rank_label).c_str()); WChessPrint(L" ");
#endif

        for (int col = 0; col < CHESS_BOARD_WIDTH; ++col) {
#ifdef _WIN32
            WChessPrint("| ");
#else
            WChessPrint(L"| ");
#endif

            // Determine the bit index for the current square based on display orientation
            int bit_index;
            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo)) {
                // Flipped board (Player Two's perspective)
                // Row 0 corresponds to rank 1, file 0 corresponds to file 'h'
                bit_index = row_col_to_bit_index(row + 1, CHESS_BOARD_WIDTH - 1 - col);
            } else {
                // Standard board (Player One's perspective)
                // Row 0 corresponds to rank 8, file 0 corresponds to file 'a'
                bit_index = row_col_to_bit_index(CHESS_BOARD_HEIGHT - row, col);
            }

            SquareInfo currPosition = getSquareInfo(bit_index);

            setTerminalColor(currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.p1_color : GameOptions.p2_color);
            WChessPrint(TEXT_PIECE_ART_COLLECTION[currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.whitePlayerArtSelector : GameOptions.blackPlayerArtSelector][static_cast<int>(currPosition.piece)]);
            setTerminalColor(DEFAULT);
#ifdef _WIN32
            WChessPrint(" ");
#else
            WChessPrint(L" ");
#endif
        }
#ifdef _WIN32
        WChessPrint("| "); WChessPrint(std::to_string(rank_label).c_str()); WChessPrint("\n");
        WChessPrint("\t\t\t  +---+---+---+---+---+---+---+---+\n");
#else
        WChessPrint(L"| "); WChessPrint(std::to_wstring(rank_label).c_str()); WChessPrint(L"\n");
        WChessPrint(L"\t\t\t  +---+---+---+---+---+---+---+---+\n");
#endif
    }
#ifdef _WIN32
    WChessPrint("\t\t\t    a   b   c   d   e   f   g   h\n");
#else
    WChessPrint(L"\t\t\t    a   b   c   d   e   f   g   h\n");
#endif

    if (!toPrint.empty()) {
#ifdef _WIN32
        WChessPrint(toPrint.c_str());
#else
        WChessPrint(std::wstring(toPrint.begin(), toPrint.end()).c_str());
#endif
        toPrint.clear();
    }
}


//! This may have to have a std::wstring version for linux
// -1 invalid input
// 0 options
// 1 valid input
ChessEnums::SanitizeGetMoveResult StandardChessGame::sanitizeGetMove(std::string& input) {

    if(!std::iswalpha(input[0])) 
        return ChessEnums::SanitizeGetMoveResult::Invalid; // Ensure [0] is alphabetical character
    
    // Force to lower case
    input[0] = std::towlower(input[0]); // Force [0] to lower case
    switch(input[0]) {
        case 'a': case 'b': case 'c': case 'd':
        case 'e': case 'f': case 'g': case 'h':
            break;
        case 'q': // Valid too but for option
        case 'x': 
        case 'o':
            return ChessEnums::SanitizeGetMoveResult::Options;
        case 'r':
            return ChessEnums::SanitizeGetMoveResult::ReEnterMove;
        case 'p':
            return ChessEnums::SanitizeGetMoveResult::RePrintBoard;
        default: // Ensure one of the characters above
            return ChessEnums::SanitizeGetMoveResult::Invalid; // Ensure [0] is alphabetical character
    }

    // Make sure length is 2
    if(input.length() != 2)
        return ChessEnums::SanitizeGetMoveResult::Invalid; // Move must be length of 2

    // Ensure 2nd char is a number
    if(!std::isdigit(input[1]) || input[1] == '0' || input[1] == '9') // Using std::iswdigit
        return ChessEnums::SanitizeGetMoveResult::Invalid; // Ensure [1] must be a digit and not '0' or '9

    // Now we know it must be [0] == a-h, [1] == 1 - 8
    return ChessEnums::SanitizeGetMoveResult::Valid;
} 

std::string StandardChessGame::playerToString(ChessTypes::Player p){
    return (p == ChessTypes::Player::PlayerOne ? "Player One" : "Player Two");
}

// 0 FREE
// 1 PONE TAKEN
// 2 PTWO TAKEN
ChessTypes::Owner StandardChessGame::piecePresent(Point p){
    SquareInfo position = getSquareInfo(p.m_y * 8 + p.m_x);
    if(position.owner == ChessTypes::Owner::None && position.piece == ChessTypes::GamePiece::None)
        return ChessTypes::Owner::None;
    return position.owner;    
}

void StandardChessGame::printBoardWithMoves() {
    if(GameOptions.clearScreenOnPrint)
        eraseDisplay();

}

HalfMove StandardChessGame::convertMove(std::string move){

    int row = 8 - (move[1] - 49);
    int col = move[0] - 96;
    int bit_index = row_col_to_bit_index(row, col);

    //! TODO: Add logic for reflection ~ Check if this is correct
    if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) ||
    (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
        return HalfMove(bit_index, getSquareInfo(row_col_to_bit_index(8 - row, 7 - col)));

    return HalfMove(bit_index, getSquareInfo(bit_index));
}


// -1 Puts king in harm way
// 0 Invalid move
// 1 Piece taken
// 2 Piece moved
ChessEnums::MakeMoveResult StandardChessGame::makeMove(Move&& move){
    return ChessEnums::MakeMoveResult::PieceMoved;
}

// True valid move
// False invalid move
bool StandardChessGame::verifyMove(){ return false; }

// True - Good, clear path
// False - Something blocking
bool StandardChessGame::rookClearPath(){ return false; }

// True - Good, clear path
// False - Something blocking
bool StandardChessGame::bishopClearPath(){ return false; }

// True - Valid move for pawn
// False - Invalid move for pawn
bool StandardChessGame::pawnMoveCheck(){ return false; }

// True - All good
// False - Piece in way
bool StandardChessGame::unobstructedPathCheck(){ return false; }

// True - can defend the king so not checkmate
// False - checkmate!
// if it is a queen causing check, called will temporarily change piece causing check to rook, then change it to bishop then change it back
bool StandardChessGame::canDefendKing() { return false; }

// If nullptr is passed then it will check for general king saftey against every enemy piece
// If a pointer is passed it will check for saftey against the gamesquare passed
// True - King safe
// False - King NOT safe
bool StandardChessGame::kingSafe() { return false; }

bool StandardChessGame::kingSafeAfterMove() { return false; }

// True Gameover, Current turn loses
// False the king can get out of check
bool StandardChessGame::checkMate(){ return false; }

