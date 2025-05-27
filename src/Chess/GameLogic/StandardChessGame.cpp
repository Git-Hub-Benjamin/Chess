#include <random>
#include <cwctype>

#include "../../Util/Terminal/Terminal.hpp"
#include "../TypesAndEnums/ChessTypes.hpp"
#include "../TypesAndEnums/ChessEnums.hpp"
#include "../StandardChessGame.hpp"
#include "../Utils/ChessConstants.hpp"
#include "../../Util/Terminal/TextPieceArt.hpp"
#include "../Utils/ChessHelperFunctions.hpp"

SquareInfo StandardChessGame::getSquareInfo(int row, int col) {
    int bit_index = row * 8 + col;
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

int rank_file_to_bit_index(int, int);
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
        set_bit(white_pawns, rank_file_to_bit_index(2, file));
    }

    // White Rooks: a1 and h1 (Rank 1, files 0 and 7)
    set_bit(white_rooks, rank_file_to_bit_index(1, 0));
    set_bit(white_rooks, rank_file_to_bit_index(1, 7));

    // White Knights: b1 and g1 (Rank 1, files 1 and 6)
    set_bit(white_knights, rank_file_to_bit_index(1, 1));
    set_bit(white_knights, rank_file_to_bit_index(1, 6));

    // White Bishops: c1 and f1 (Rank 1, files 2 and 5)
    set_bit(white_bishops, rank_file_to_bit_index(1, 2));
    set_bit(white_bishops, rank_file_to_bit_index(1, 5));

    // White Queen: d1 (Rank 1, file 3)
    set_bit(white_queens, rank_file_to_bit_index(1, 3));

    // White King: e1 (Rank 1, file 4)
    set_bit(white_king, rank_file_to_bit_index(1, 4));

    // 3. Set bits for Black pieces

    // Black Pawns: Rank 7 (row 6)
    for (int file = 0; file < 8; ++file) {
        set_bit(black_pawns, rank_file_to_bit_index(7, file));
    }

    // Black Rooks: a8 and h8 (Rank 8, files 0 and 7)
    set_bit(black_rooks, rank_file_to_bit_index(8, 0));
    set_bit(black_rooks, rank_file_to_bit_index(8, 7));

    // Black Knights: b8 and g8 (Rank 8, files 1 and 6)
    set_bit(black_knights, rank_file_to_bit_index(8, 1));
    set_bit(black_knights, rank_file_to_bit_index(8, 6));

    // Black Bishops: c8 and f8 (Rank 8, files 2 and 5)
    set_bit(black_bishops, rank_file_to_bit_index(8, 2));
    set_bit(black_bishops, rank_file_to_bit_index(8, 5));

    // Black Queen: d8 (Rank 8, file 3)
    set_bit(black_queens, rank_file_to_bit_index(8, 3));

    // Black King: e8 (Rank 8, file 4)
    set_bit(black_king, rank_file_to_bit_index(8, 4));

    // 4. Calculate occupancy bitboards
    white_occupancy = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_king;
    black_occupancy = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_king;
    all_occupancy = white_occupancy | black_occupancy;
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

    } else {
        if(static_cast<ChessTypes::Player>(square.owner) == currentTurn)
            return ChessEnums::ValidateGameSquareResult::CANNOT_TAKE_OWN;
            //! This does NOT account for castling or special moves
    }
    
    return ChessEnums::ValidateGameSquareResult::VALID;
}

void StandardChessGame::printBoard(ChessTypes::Player playerSideToPrint){
    if (GameOptions.clearScreenOnPrint) 
        eraseDisplay();
#ifdef _WIN32
    std::string board;
    board += "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n";
    board += "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        board += "\t\t\t" + std::to_string(CHESS_BOARD_HEIGHT - row) + " ";
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            board += "| ";
            wchar_t piece;
            SquareInfo currPosition; 

            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && playerSideToPrint == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && playerSideToPrint == ChessTypes::Player::PlayerTwo))
                currPosition = getSquareInfo(CHESS_BOARD_HEIGHT - row, CHESS_BOARD_WIDTH - col); // <-- IDK IF THESE ARE RIGHT
            else
                currPosition = getSquareInfo(row * 8, col); // <-- IDK IF THESE ARE RIGHT


            board += TEXT_PIECE_ART_COLLECTION[currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.whitePlayerArtSelector : GameOptions.blackPlayerArtSelector][static_cast<int>(currPosition.piece)];
            //! TODO: WINDOWS COLOR
            //setTerminalColor(currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.p1_color : GameOptions.p2_color);

            setTerminalColor(DEFAULT);
            board += " ";
        }
        board += "| " + std::to_string(CHESS_BOARD_HEIGHT - row) + "\n";
        board += "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    }
    board += "\t\t\t    a   b   c   d   e   f   g   h\n";

    if (!toPrint.empty()) {
        board += toPrint;
        toPrint.clear();
    }

    WChessPrint(board.c_str());
#else

    std::wstring board;
    board += L"\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n";
    board += L"\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        board += L"\t\t\t";
        board += std::to_wstring(CHESS_BOARD_HEIGHT - row) + L" ";
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            board += L"| ";
            wchar_t piece;
            SquareInfo currPosition; 

            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && playerSideToPrint == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && playerSideToPrint == ChessTypes::Player::PlayerTwo))
                currPosition = getSquareInfo(CHESS_BOARD_HEIGHT - row, CHESS_BOARD_WIDTH - col); // <-- IDK IF THESE ARE RIGHT
            else
                currPosition = getSquareInfo(row * 8, col); // <-- IDK IF THESE ARE RIGHT

            //! TODO: FIX LINUX
            //board += TEXT_PIECE_ART_COLLECTION[currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.whitePlayerArtSelector : GameOptions.blackPlayerArtSelector][static_cast<int>(currPosition.piece)];
            setTerminalColor(currPosition.owner == ChessTypes::Owner::PlayerOne ? GameOptions.p1_color : GameOptions.p2_color);

            setTerminalColor(DEFAULT);
            board += L" ";
        }
        board += L"| ";
        board += std::to_wstring(CHESS_BOARD_HEIGHT - row) + L"\n";
        board += L"\t\t\t  +---+---+---+---+---+---+---+---+\n";
    }
    board += L"\t\t\t    a   b   c   d   e   f   g   h\n";

    if (!toPrint.empty()) {
        board += convertString(toPrint);
        toPrint.clear();
    }

    WChessPrint(board.c_str());
#endif
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
    SquareInfo position = getSquareInfo(p.m_y, p.m_x);
    if(position.owner == ChessTypes::Owner::None && position.piece == ChessTypes::GamePiece::None)
        return ChessTypes::Owner::None;
    return position.owner;    
}

void StandardChessGame::printBoardWithMoves(ChessTypes::Player playerSideToPrint) {
    if(GameOptions.clearScreenOnPrint)
        eraseDisplay();
/*
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - row << " ";
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){

            std::wcout << "| ";
            wchar_t piece;
            ChessEnums::PossibleMovesResult possibleMoveTypeSelector;
            SquareInfo currPosition = getSquareInfo(row * 8, col); 

            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && playerSideToPrint == ChessTypes::Player::PlayerTwo) 
            || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && playerSideToPrint == ChessTypes::Player::PlayerTwo)) {
                  if(currPosition.owner == ChessTypes::Owner::None)
                    piece = ' ';
                else if(currPosition.owner == ChessTypes::Owner::PlayerOne){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][static_cast<int>(currPosition.piece)];
                    setTerminalColor(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][static_cast<int>(currPosition.piece)];
                    setTerminalColor(GameOptions.p2_color);
                }

                // checking if the current square can be acctacked by piece
                //! TODO:
                //possibleMoveTypeSelector = readPossibleMoves();
                
            } else {
                if(currPosition.owner == ChessTypes::Owner::None)
                    piece = ' ';
                else if(currPosition.owner == ChessTypes::Owner::PlayerOne){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][static_cast<int>(currPosition.piece)];
                    setTerminalColor(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][static_cast<int>(currPosition.piece)];
                    setTerminalColor(GameOptions.p2_color);
                }

                // checking if the current square can be acctacked by piece
                //! TODO:
                //possibleMoveTypeSelector = readPossibleMoves();
            }

            if(possibleMoveTypeSelector != ChessEnums::PossibleMovesResult::NOT_FOUND){
                if(piece == ' ')
                    piece = 'X';
                WRITE_COLOR color;
                switch(possibleMoveTypeSelector) {
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_ENEMY_PIECE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_OPEN_SQAURE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_PROTECT_KING_PIECE:
                        color = WRITE_COLOR::BRIGHT_MAGENTA;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_PROTECT_KING_SQUARE:
                        color = WRITE_COLOR::BRIGHT_YELLOW;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_KING_IN_DANGER:
                        color = WRITE_COLOR::YELLOW;
                        break;
                    default:
                        color = WRITE_COLOR::BLACK;
                        break;
                }
                setTerminalColor(color);
            }

            // Checking for highlighted piece, if flip board option is on then we need to look at the board the opposite way
            // otherwise if flip board is not active then we can always read it 0 - 7, 0 - 7
            //! TODO:
            
            // if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == PlayerOne && fromHighlightedPiece == &GameBoard[row][col]) ||
            //     currentTurn == PlayerTwo && fromHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
            //     !GameOptions.flipBoardOnNewTurn && (fromHighlightedPiece == &GameBoard[row][col]))
            //     setTerminalColor(GameOptions.movingPiece_color);

            // // Exact same thing as above but excpet now checking for toHighlightedPiece
            // else if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == PlayerOne && toHighlightedPiece == &GameBoard[row][col]) ||
            //     currentTurn == PlayerTwo && toHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
            //     !GameOptions.flipBoardOnNewTurn && (toHighlightedPiece == &GameBoard[row][col]))
                // setTerminalColor(GameOptions.movingToPiece_color);
            
            std::wcout << piece;    
            setTerminalColor(DEFAULT);
            std::wcout << " ";
              
        }
        std::wcout << "| " << CHESS_BOARD_HEIGHT - row << std::endl;
        std::wcout << "\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t    a   b   c   d   e   f   g   h\n";
    if (!toPrint.empty()) {
        std::wcout << toPrint << std::endl;
        toPrint.clear();
    }
*/
}

uint64_t StandardChessGame::convertMove(std::wstring move, ChessTypes::Player sideToConvert){
    // Convert letter to number (a = 0, b = 1 etc)
    int col = move[0] - L'a';

    // Convert char number to number ('1' = 1, '8' = 8 etc)
    // Subtract 1 because rank 1 corresponds to row 0
    int rank = move[1] - L'0';
    int row = rank - 1;

    //! TODO 
    // if ((GameConnectivity == ONLINE_CONNECTIVITY && sideToConvert == PlayerTwo) ||
    // (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && sideToConvert == PlayerTwo))
    //     return GameBoard[reflectAxis(row)][reflectAxis(col)];

    return static_cast<uint64_t>(row * 8 + col);
}


// -1 Puts king in harm way
// 0 Invalid move
// 1 Piece taken
// 2 Piece moved
int StandardChessGame::makeMove(){ return false; }

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

