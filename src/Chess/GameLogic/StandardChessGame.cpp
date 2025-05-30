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
#include "../Utils/Movesets.hpp"

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

void StandardChessGame::printBitboards(){
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

void StandardChessGame::initGame(){
    white_pawns = 0; white_knights = 0; white_bishops = 0;
    white_rooks = 0; white_queens = 0; white_king = 0;
    black_pawns = 0; black_knights = 0; black_bishops = 0;
    black_rooks = 0; black_queens = 0; black_king = 0;
    white_occupancy = 0; black_occupancy = 0; all_occupancy = 0;

    // 2. Set bits for White pieces

    // White Pawns: Rank 2 (row 1)
    for (int file = 0; file < 8; ++file) {
        set_bit(white_pawns, row_col_to_bit_index(6, file));
    }

    // White Rooks: a1 and h1 (Rank 1, files 0 and 7)
    set_bit(white_rooks, row_col_to_bit_index(7, 0));
    set_bit(white_rooks, row_col_to_bit_index(7, 7));

    // White Knights: b1 and g1 (Rank 1, files 1 and 6)
    set_bit(white_knights, row_col_to_bit_index(7, 1));
    set_bit(white_knights, row_col_to_bit_index(7, 6));

    // White Bishops: c1 and f1 (Rank 1, files 2 and 5)
    set_bit(white_bishops, row_col_to_bit_index(7, 2));
    set_bit(white_bishops, row_col_to_bit_index(7, 5));

    // White Queen: d1 (Rank 1, file 3)
    set_bit(white_queens, row_col_to_bit_index(7, 3));

    // White King: e1 (Rank 1, file 4)
    set_bit(white_king, row_col_to_bit_index(7, 4));

    // 3. Set bits for Black pieces

    // Black Pawns: Rank 7 (row 6)
    for (int file = 0; file < 8; ++file) {
        set_bit(black_pawns, row_col_to_bit_index(1, file));
    }

    // Black Rooks: a8 and h8 (Rank 8, files 0 and 7)
    set_bit(black_rooks, row_col_to_bit_index(0, 0));
    set_bit(black_rooks, row_col_to_bit_index(0, 7));

    // Black Knights: b8 and g8 (Rank 8, files 1 and 6)
    set_bit(black_knights, row_col_to_bit_index(0, 1));
    set_bit(black_knights, row_col_to_bit_index(0, 6));

    // Black Bishops: c8 and f8 (Rank 8, files 2 and 5)
    set_bit(black_bishops, row_col_to_bit_index(0, 2));
    set_bit(black_bishops, row_col_to_bit_index(0, 5));

    // Black Queen: d8 (Rank 8, file 3)
    set_bit(black_queens, row_col_to_bit_index(0, 3));

    // Black King: e8 (Rank 8, file 4)
    set_bit(black_king, row_col_to_bit_index(0, 4));

    // 4. Calculate occupancy bitboards
    white_occupancy = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_king;
    black_occupancy = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_king;
    all_occupancy = white_occupancy | black_occupancy;
}


bool StandardChessGame::populatePossibleMoves(HalfMove& fromSquare) { 
    
    for (int i = 0; i < 64; i++) {
        HalfMove toSquare(i, getSquareInfo(i));
        Move move(fromSquare, toSquare);
        if (verifyMove(move)) 
            possibleMoves.push_back(i);
    }

    return possibleMoves.size() > 0;
}


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
            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
                bit_index = row_col_to_bit_index(CHESS_BOARD_HEIGHT - row - 1, CHESS_BOARD_WIDTH - col - 1); // player two's perspective
            else 
                bit_index = row_col_to_bit_index(row, col); // player one's perspective

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


//! TODO: fix bug with possiblemovesvector printing on reflected side too?
void StandardChessGame::printBoardWithMoves() {
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
            std::string piece;
#else 
            WChessPrint(L"| ");
            std::wstring piece;
#endif
            WRITE_COLOR color = DEFAULT;
            int bit_index;

            // find bit index based on perspective
            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
                bit_index = row_col_to_bit_index(CHESS_BOARD_HEIGHT - row - 1, CHESS_BOARD_WIDTH - col - 1); // player two's perspective
            else 
                bit_index = row_col_to_bit_index(row, col); // player one's perspective

            SquareInfo currPosition = getSquareInfo(bit_index);

            piece = TEXT_PIECE_ART_COLLECTION[currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.whitePlayerArtSelector : GameOptions.blackPlayerArtSelector][static_cast<int>(currPosition.piece)];
            

            if (readPossibleMoves(bit_index)) {
#ifdef _WIN32
                if (piece == " ")
                    piece = "X";
#else 
                if (piece == L" ")
                    piece = L"X";
#endif
                if (currPosition.owner == (currentTurn == ChessTypes::Player::PlayerOne ? ChessTypes::Owner::PlayerTwo : ChessTypes::Owner::PlayerOne) || currPosition.owner == ChessTypes::Owner::None)
                    color = GameOptions.possibleMove_color;
                else //^ DEBUGGING
                    color = WRITE_COLOR::YELLOW;

            }

            // This is so special color highlighting, i just dont know if i want to keep it yet
            /*
            if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == ChessTypes::Player::PlayerOne && fromHighlightedPiece == &GameBoard[row][col]) || currentTurn == ChessTypes::Player::PlayerTwo && fromHighlightedPiece == &GameBoard[7 - row][7 - col])) || !GameOptions.flipBoardOnNewTurn && (fromHighlightedPiece == &GameBoard[row][col]))
                color = GameOptions.movingPiece_color;

            // Exact same thing as above but excpet now checking for toHighlightedPiece
            else if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == ChessTypes::Player::PlayerOne && toHighlightedPiece == &GameBoard[row][col]) ||
                                                         currentTurn == ChessTypes::Player::PlayerTwo && toHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
                     !GameOptions.flipBoardOnNewTurn && (toHighlightedPiece == &GameBoard[row][col]))
                color = GameOptions.movingToPiece_color;
            */

            if (color == DEFAULT)
                if (currPosition.owner == ChessTypes::Owner::PlayerOne)
                    color = GameOptions.p1_color;
                else
                    color = GameOptions.p2_color;

            setTerminalColor(color);
            WChessPrint(piece.c_str());
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

bool StandardChessGame::readPossibleMoves(int bit_index) {
    for (int i = 0; i < possibleMoves.size(); i++) {
        if (possibleMoves[i] == bit_index) {
            return true;
        }
    }
    return false;
}

HalfMove StandardChessGame::convertMove(std::string move){

    int row = 8 - (move[1] - 48);
    int col = tolower(move[0]) - 'a';

    //! TODO: Add logic for reflection ~ Check if this is correct
    if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) ||
    (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
        return HalfMove(move, row_col_to_bit_index(7 - row, 7 - col), getSquareInfo(row_col_to_bit_index(7 - row, 7 - col)));

    return HalfMove(move, row_col_to_bit_index(row, col), getSquareInfo(row_col_to_bit_index(row, col)));
}


ChessEnums::MakeMoveResult StandardChessGame::movePiece(Move& move){
    // Get the current piece's info
    SquareInfo fromSquare = move.from.square;
    ChessTypes::Owner owner = fromSquare.owner;
    ChessTypes::GamePiece piece = fromSquare.piece;

    // Clear the piece from its original position
    if (owner == ChessTypes::Owner::PlayerOne) {
        // Clear piece from specific white piece bitboard
        switch (piece) {
            case ChessTypes::GamePiece::Pawn:
                white_pawns &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Knight:
                white_knights &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Bishop:
                white_bishops &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Rook:
                white_rooks &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Queen:
                white_queens &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::King:
                white_king &= ~(1ULL << move.from.bit_index);
                break;
        }
        white_occupancy &= ~(1ULL << move.from.bit_index);
    } else {
        // Clear piece from specific black piece bitboard
        switch (piece) {
            case ChessTypes::GamePiece::Pawn:
                black_pawns &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Knight:
                black_knights &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Bishop:
                black_bishops &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Rook:
                black_rooks &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::Queen:
                black_queens &= ~(1ULL << move.from.bit_index);
                break;
            case ChessTypes::GamePiece::King:
                black_king &= ~(1ULL << move.from.bit_index);
                break;
        }
        black_occupancy &= ~(1ULL << move.from.bit_index);
    }

    // If there's a piece at the destination, remove it first
    if (move.to.square.owner != ChessTypes::Owner::None) {
        // Clear captured piece from opponent's bitboards
        if (move.to.square.owner == ChessTypes::Owner::PlayerOne) {
            switch (move.to.square.piece) {
                case ChessTypes::GamePiece::Pawn:
                    white_pawns &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Knight:
                    white_knights &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Bishop:
                    white_bishops &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Rook:
                    white_rooks &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Queen:
                    white_queens &= ~(1ULL << move.to.bit_index);
                    break;
            }
            white_occupancy &= ~(1ULL << move.to.bit_index);
        } else {
            switch (move.to.square.piece) {
                case ChessTypes::GamePiece::Pawn:
                    black_pawns &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Knight:
                    black_knights &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Bishop:
                    black_bishops &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Rook:
                    black_rooks &= ~(1ULL << move.to.bit_index);
                    break;
                case ChessTypes::GamePiece::Queen:
                    black_queens &= ~(1ULL << move.to.bit_index);
                    break;
            }
            black_occupancy &= ~(1ULL << move.to.bit_index);
        }
    }

    // Place the piece in its new position
    if (owner == ChessTypes::Owner::PlayerOne) {
        // Set piece in specific white piece bitboard
        switch (piece) {
            case ChessTypes::GamePiece::Pawn:
                white_pawns |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Knight:
                white_knights |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Bishop:
                white_bishops |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Rook:
                white_rooks |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Queen:
                white_queens |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::King:
                white_king |= (1ULL << move.to.bit_index);
                break;
        }
        white_occupancy |= (1ULL << move.to.bit_index);
    } else {
        // Set piece in specific black piece bitboard
        switch (piece) {
            case ChessTypes::GamePiece::Pawn:
                black_pawns |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Knight:
                black_knights |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Bishop:
                black_bishops |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Rook:
                black_rooks |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::Queen:
                black_queens |= (1ULL << move.to.bit_index);
                break;
            case ChessTypes::GamePiece::King:
                black_king |= (1ULL << move.to.bit_index);
                break;
        }
        black_occupancy |= (1ULL << move.to.bit_index);
    }

    // Update all occupancy
    all_occupancy = white_occupancy | black_occupancy;

    return move.to.square.owner != ChessTypes::Owner::None ? 
           ChessEnums::MakeMoveResult::PieceTaken : 
           ChessEnums::MakeMoveResult::PieceMoved;

}

bool StandardChessGame::testKingSafe(Move& move){
    uint64_t old_white_pawns = white_pawns;
    uint64_t old_white_knights = white_knights;
    uint64_t old_white_bishops = white_bishops;
    uint64_t old_white_rooks = white_rooks;
    uint64_t old_white_queens = white_queens;
    uint64_t old_white_king = white_king;
    uint64_t old_black_pawns = black_pawns;
    uint64_t old_black_knights = black_knights;
    uint64_t old_black_bishops = black_bishops;
    uint64_t old_black_rooks = black_rooks;
    uint64_t old_black_queens = black_queens;
    uint64_t old_black_king = black_king;
    uint64_t old_white_occupancy = white_occupancy;
    uint64_t old_black_occupancy = black_occupancy;
    uint64_t old_all_occupancy = all_occupancy;

    // Make the move
    ChessEnums::MakeMoveResult result = movePiece(move);

    // Check if the king is still in check
    if (!kingSafe()) {
        white_pawns = old_white_pawns;
        white_knights = old_white_knights;
        white_bishops = old_white_bishops;
        white_rooks = old_white_rooks;
        white_queens = old_white_queens;
        white_king = old_white_king;
        black_pawns = old_black_pawns;
        black_knights = old_black_knights;
        black_bishops = old_black_bishops;
        black_rooks = old_black_rooks;
        black_queens = old_black_queens;
        black_king = old_black_king;
        white_occupancy = old_white_occupancy;
        black_occupancy = old_black_occupancy;
        all_occupancy = old_all_occupancy;
        return false;
    }

    return true;

}

// -1 Puts king in harm way
// 0 Invalid move
// 1 Piece taken
// 2 Piece moved
ChessEnums::MakeMoveResult StandardChessGame::makeMove(Move& move){

    if(!verifyMove(move)){
        return ChessEnums::MakeMoveResult::InvalidMove;
    }

    // if(isPawnPromotion()){
    //     return ChessEnums::MakeMoveResult::PawnPromotion;
    // }

    if (currTurnInCheck) 
        if (!testKingSafe(move)) 
            return ChessEnums::MakeMoveResult::KingInDanger;

    return movePiece(move);
}

bool StandardChessGame::isPawnPromotion(Move& move) {
    if (move.from.square.piece == ChessTypes::GamePiece::Pawn && (move.from.square.owner == ChessTypes::Owner::PlayerOne && move.to.bit_index / 8 == 0 || move.from.square.owner == ChessTypes::Owner::PlayerTwo && move.to.bit_index / 8 == 7))
        return true;
    
    return false;
}

bool StandardChessGame::verifyMove(Move& move) { 
    // First check if the move is valid for the piece type
    if (!validateMoveset(move)) {
        return false;
    }

    // Then check if the path is clear (except for knights)
    if (!unobstructedPathCheck(move)) {
        return false;
    }

    // Finally check if the move doesn't put/leave own king in check
    if (!kingSafe()) {
        return false;
    }

    return true;
}

// True - valid moveset
// False - invalid moveset
bool StandardChessGame::validateMoveset(Move& move) {
    SquareInfo fromSquare = getSquareInfo(move.from.bit_index);
    ChessTypes::GamePiece fromPiece = fromSquare.piece;
    
    // Convert bit indices to board coordinates
    int fromRow = move.from.bit_index / 8;
    int fromCol = move.from.bit_index % 8;
    int toRow = move.to.bit_index / 8;
    int toCol = move.to.bit_index % 8;

    // Get the piece's possible moves count and moveset pointer
    int pieceIndex;
    if (fromPiece == ChessTypes::GamePiece::Pawn) {
        // Use PawnDown for black pawns (PlayerTwo), PawnUp for white pawns (PlayerOne)
        pieceIndex = (currentTurn == ChessTypes::Player::PlayerTwo) ? 0 : 1;
    } else {
        // For other pieces, use their regular index
        pieceIndex = static_cast<int>(fromPiece) + 1; // +1 to skip the two pawn movesets
    }

    // Check each possible move for the piece
    short (*moveSet)[2] = PieceMovePtrs[pieceIndex];
    int moveCount = PieceMoveCounts[static_cast<int>(fromPiece) - 1];

    for (int move_set_count = 0; move_set_count < moveCount; move_set_count++) {
        int newRow = fromRow + moveSet[move_set_count][1];
        int newCol = fromCol + moveSet[move_set_count][0];
        
        // If this possible move matches our target position
        if (newRow == toRow && newCol == toCol) {
            // For pawns, we need additional validation
            if (fromPiece == ChessTypes::GamePiece::Pawn) {
                // Regular move (1 square forward)
                if (move_set_count == 0) {
                    return !((all_occupancy >> move.to.bit_index) & 1); // Must be empty
                }
                // Capture moves (diagonal)
                else if (move_set_count == 1 || move_set_count == 2) {
                    uint64_t enemy_pieces = (currentTurn == ChessTypes::Player::PlayerOne) ? 
                                          black_occupancy : white_occupancy;
                    return (enemy_pieces >> move.to.bit_index) & 1; // Must capture enemy piece
                }
                // Double move (2 squares forward)
                else if (move_set_count == 3) {
                    // Check if it's the pawn's first move
                    bool isFirstMove = (currentTurn == ChessTypes::Player::PlayerOne && fromRow == 6) ||
                                     (currentTurn == ChessTypes::Player::PlayerTwo && fromRow == 1);
                    if (!isFirstMove) return false;
                    
                    // Check if both squares are empty
                    int midSquare = (fromRow + moveSet[0][1]) * 8 + fromCol; // One square forward
                    return !((all_occupancy >> midSquare) & 1) && 
                           !((all_occupancy >> move.to.bit_index) & 1);
                }
            }
            return true;
        }
    }

    return false;
}

// True - All good
// False - Piece in way
bool StandardChessGame::unobstructedPathCheck(Move& move) {
    SquareInfo fromSquare = getSquareInfo(move.from.bit_index);
    
    // Knights can jump over pieces
    if (fromSquare.piece == ChessTypes::GamePiece::Knight) {
        return true;
    }

    // For other pieces, check the path
    switch (fromSquare.piece) {
        case ChessTypes::GamePiece::Rook:
            return rookClearPath(move);
        case ChessTypes::GamePiece::Bishop:
            return bishopClearPath(move);
        case ChessTypes::GamePiece::Queen:
            // Queen moves like rook or bishop
            if (move.from.bit_index / 8 == move.to.bit_index / 8 || 
                move.from.bit_index % 8 == move.to.bit_index % 8) {
                return rookClearPath(move);
            } else {
                return bishopClearPath(move);
            }
        case ChessTypes::GamePiece::Pawn:
            return pawnMoveCheck(move);
        default:
            return true; // King only moves one square, no need to check path
    }
}

// True - Good, clear path
// False - Something blocking
bool StandardChessGame::rookClearPath(Move& move) {
    int fromRow = move.from.bit_index / 8;
    int fromCol = move.from.bit_index % 8;
    int toRow = move.to.bit_index / 8;
    int toCol = move.to.bit_index % 8;

    // Moving along a rank
    if (fromRow == toRow) {
        int step = (toCol > fromCol) ? 1 : -1;
        for (int col = fromCol + step; col != toCol; col += step) {
            if ((all_occupancy >> (fromRow * 8 + col)) & 1) {
                return false;
            }
        }
    }
    // Moving along a file
    else if (fromCol == toCol) {
        int step = (toRow > fromRow) ? 1 : -1;
        for (int row = fromRow + step; row != toRow; row += step) {
            if ((all_occupancy >> (row * 8 + fromCol)) & 1) {
                return false;
            }
        }
    }

    return true;
}

// True - Good, clear path
// False - Something blocking
bool StandardChessGame::bishopClearPath(Move& move) {
    int fromRow = move.from.bit_index / 8;
    int fromCol = move.from.bit_index % 8;
    int toRow = move.to.bit_index / 8;
    int toCol = move.to.bit_index % 8;

    int rowStep = (toRow > fromRow) ? 1 : -1;
    int colStep = (toCol > fromCol) ? 1 : -1;

    int row = fromRow + rowStep;
    int col = fromCol + colStep;

    while (row != toRow && col != toCol) {
        if ((all_occupancy >> (row * 8 + col)) & 1) {
            return false;
        }
        row += rowStep;
        col += colStep;
    }

    return true;
}

// True - Valid move for pawn
// False - Invalid move for pawn
bool StandardChessGame::pawnMoveCheck(Move& move) {
    int fromRow = move.from.bit_index / 8;
    int fromCol = move.from.bit_index % 8;
    int toRow = move.to.bit_index / 8;
    int toCol = move.to.bit_index % 8;

    // Get direction based on color
    int direction = (currentTurn == ChessTypes::Player::PlayerOne) ? -1 : 1;

    // Moving straight
    if (fromCol == toCol) {
        // Single square advance
        if (toRow == fromRow + direction) {
            return !((all_occupancy >> move.to.bit_index) & 1);
        }
        // Double square advance from starting position
        if ((currentTurn == ChessTypes::Player::PlayerOne && fromRow == 6) ||
            (currentTurn == ChessTypes::Player::PlayerTwo && fromRow == 1)) {
            if (toRow == fromRow + 2 * direction) {
                // Check both squares are empty
                int midSquare = (fromRow + direction) * 8 + fromCol;
                return !((all_occupancy >> midSquare) & 1) && 
                       !((all_occupancy >> move.to.bit_index) & 1);
            }
        }
    }
    // Diagonal capture
    else if (std::abs(toCol - fromCol) == 1 && toRow == fromRow + direction) {
        // Must capture an enemy piece
        uint64_t enemy_pieces = (currentTurn == ChessTypes::Player::PlayerOne) ? 
                               black_occupancy : white_occupancy;
        return (enemy_pieces >> move.to.bit_index) & 1;
    }

    return false;
}

// True - can defend the king so not checkmate
// False - checkmate!
// if it is a queen causing check, called will temporarily change piece causing check to rook, then change it to bishop then change it back
bool StandardChessGame::canDefendKing() { return false; }

// If nullptr is passed then it will check for general king saftey against every enemy piece
// If a pointer is passed it will check for saftey against the gamesquare passed
// True - King safe
// False - King NOT safe
bool StandardChessGame::kingSafe() { 

    // Get current player's king position
    uint64_t king = (currentTurn == ChessTypes::Player::PlayerOne) ? white_king : black_king;
    int king_pos = __builtin_ctzll(king); // Get least significant 1 bit position
    
    int kingRow = king_pos / 8;
    int kingCol = king_pos % 8;

    // Get enemy pieces
    uint64_t enemy_pawns = (currentTurn == ChessTypes::Player::PlayerOne) ? black_pawns : white_pawns;
    uint64_t enemy_knights = (currentTurn == ChessTypes::Player::PlayerOne) ? black_knights : white_knights;
    uint64_t enemy_bishops = (currentTurn == ChessTypes::Player::PlayerOne) ? black_bishops : white_bishops;
    uint64_t enemy_rooks = (currentTurn == ChessTypes::Player::PlayerOne) ? black_rooks : white_rooks;
    uint64_t enemy_queens = (currentTurn == ChessTypes::Player::PlayerOne) ? black_queens : white_queens;
    uint64_t enemy_king = (currentTurn == ChessTypes::Player::PlayerOne) ? black_king : white_king;

    // Check pawn attacks
    int pawn_direction = (currentTurn == ChessTypes::Player::PlayerOne) ? 1 : -1;
    if (kingRow + pawn_direction >= 0 && kingRow + pawn_direction < 8) {
        if (kingCol > 0) { // Check diagonal left
            int check_pos = (kingRow + pawn_direction) * 8 + (kingCol - 1);
            if ((enemy_pawns >> check_pos) & 1) return false;
        }
        if (kingCol < 7) { // Check diagonal right  
            int check_pos = (kingRow + pawn_direction) * 8 + (kingCol + 1);
            if ((enemy_pawns >> check_pos) & 1) return false;
        }
    }

    // Check knight attacks
    for (int i = 0; i < KNIGHT_POSSIBLE_MOVES; i++) {
        int newRow = kingRow + KnightMoves[i][1];
        int newCol = kingCol + KnightMoves[i][0];
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            int check_pos = newRow * 8 + newCol;
            if ((enemy_knights >> check_pos) & 1) return false;
        }
    }

    // Check diagonal attacks (bishops and queens)
    for (int i = 0; i < BISHOP_POSSIBLE_MOVES; i++) {
        int newRow = kingRow + BishopMoves[i][1];
        int newCol = kingCol + BishopMoves[i][0];
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            int check_pos = newRow * 8 + newCol;
            if (((enemy_bishops | enemy_queens) >> check_pos) & 1) {
                // Verify path is clear
                bool blocked = false;
                int stepRow = (BishopMoves[i][1] > 0) ? 1 : ((BishopMoves[i][1] < 0) ? -1 : 0);
                int stepCol = (BishopMoves[i][0] > 0) ? 1 : ((BishopMoves[i][0] < 0) ? -1 : 0);
                int r = kingRow + stepRow;
                int c = kingCol + stepCol;
                while (r != newRow || c != newCol) {
                    if ((all_occupancy >> (r * 8 + c)) & 1) {
                        blocked = true;
                        break;
                    }
                    r += stepRow;
                    c += stepCol;
                }
                if (!blocked) return false;
            }
        }
    }

    // Check straight attacks (rooks and queens)
    for (int i = 0; i < ROOK_POSSIBLE_MOVES; i++) {
        int newRow = kingRow + RookMoves[i][1];
        int newCol = kingCol + RookMoves[i][0];
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            int check_pos = newRow * 8 + newCol;
            if (((enemy_rooks | enemy_queens) >> check_pos) & 1) {
                // Verify path is clear
                bool blocked = false;
                int stepRow = (RookMoves[i][1] > 0) ? 1 : ((RookMoves[i][1] < 0) ? -1 : 0);
                int stepCol = (RookMoves[i][0] > 0) ? 1 : ((RookMoves[i][0] < 0) ? -1 : 0);
                int r = kingRow + stepRow;
                int c = kingCol + stepCol;
                while (r != newRow || c != newCol) {
                    if ((all_occupancy >> (r * 8 + c)) & 1) {
                        blocked = true;
                        break;
                    }
                    r += stepRow;
                    c += stepCol;
                }
                if (!blocked) return false;
            }
        }
    }

    // Check enemy king (kings can't be adjacent)
    for (int i = 0; i < 8; i++) { // First 8 moves are non-castling moves
        int newRow = kingRow + KingMoves[i][1];
        int newCol = kingCol + KingMoves[i][0];
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            int check_pos = newRow * 8 + newCol;
            if ((enemy_king >> check_pos) & 1) return false;
        }
    }

    return true;
}

bool StandardChessGame::kingSafeAfterMove() { return false; }

// True Gameover, Current turn loses
// False the king can get out of check
bool StandardChessGame::checkMate(){ 

    // Check if any piece can make a legal move to get out of check
    for (int fromRow = 0; fromRow < 8; fromRow++) {
        for (int fromCol = 0; fromCol < 8; fromCol++) {
            int fromIndex = fromRow * 8 + fromCol;
            
            // Skip if not current player's piece
            SquareInfo fromSquare = getSquareInfo(fromIndex);
            if (fromSquare.owner != static_cast<ChessTypes::Owner>(currentTurn)) {
                continue;
            }

            // Try all possible destination squares
            for (int toRow = 0; toRow < 8; toRow++) {
                for (int toCol = 0; toCol < 8; toCol++) {
                    int toIndex = toRow * 8 + toCol;

                    HalfMove from(fromIndex, fromSquare);
                    HalfMove to(toIndex, getSquareInfo(toIndex));
                    
                    Move testMove = {
                        from,
                        to
                    };

                    // If any valid move exists, not checkmate
                    if (verifyMove(testMove)) {
                        return false;
                    }
                }
            }
        }
    }

    // No valid moves found - checkmate
    return true;
}

