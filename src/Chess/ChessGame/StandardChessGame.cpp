#include "../chess.hpp"
#include "text-piece-art.hpp"
#include <random>

void copyStandardBoard(GameSquare from[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH], GameSquare to[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH]) {
    memcpy(to, from, sizeof(GameSquare[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH]));
}

static short PawnUp[PAWN_POSSIBLE_MOVES][2] = { {0, -1}, {1, -1}, {-1, -1}, {0, -2} };
static short PawnDown[PAWN_POSSIBLE_MOVES][2] = { {0, 1},  {-1, 1}, {1, 1}  , {0, 2}  };
static short KnightMoves[KNIGHT_POSSIBLE_MOVES][2] = { {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2} };
static short KingMoves[KING_POSSIBLE_MOVES][2] = {
    {0, 1}, {1, 1}, {1, 0}, {1, -1}, 
    {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}
};
static short BishopMoves[BISHOP_POSSIBLE_MOVES][2] = {
    {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},     // bottom left to top right
    {-1, -1}, {-2, -2}, {-3, -3}, {-4, -4}, {-5, -5}, {-6, -6}, {-7, -7},  // top right to bottom left
    {-1, 1}, {-2, 2}, {-3, 3}, {-4, 4}, {-5, 5}, {-6, 6}, {-7, 7}, // bottom right to top left
    {1, -1}, {2, -2}, {3, -3}, {4, -4}, {5, -5}, {6, -6}, {7, -7}  // top left to bottom right
};
static short RookMoves[ROOK_POSSIBLE_MOVES][2] = {
    {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},       // bottom left to bottom right
    {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},       // bottom left to top left
    {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0}, {-5, 0}, {-6, 0}, {-7, 0}, // top right to top left
    {0, -1}, {0, -2}, {0, -3}, {0, -4}, {0, -5}, {0, -6}, {0, -7}  // top right to bottom right
};
static short QueenMoves[QUEEN_POSSIBLE_MOVES][2] = {
    {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},       // Diagonal: bottom left to top right
    {-1, -1}, {-2, -2}, {-3, -3}, {-4, -4}, {-5, -5}, {-6, -6}, {-7, -7}, // Diagonal: top right to bottom left
    {-1, 1}, {-2, 2}, {-3, 3}, {-4, 4}, {-5, 5}, {-6, 6}, {-7, 7}, // Diagonal: bottom right to top left
    {1, -1}, {2, -2}, {3, -3}, {4, -4}, {5, -5}, {6, -6}, {7, -7}, // Diagonal: top left to bottom right
    {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},       // Horizontal: bottom left to bottom right
    {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},       // Vertical: bottom left to top left
    {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0}, {-5, 0}, {-6, 0}, {-7, 0}, // Horizontal: top right to top left
    {0, -1}, {0, -2}, {0, -3}, {0, -4}, {0, -5}, {0, -6}, {0, -7}  // Vertical: top right to bottom right
};

short (*pieceMovePtrs[])[2] = {
    PawnDown,
    PawnUp,
    KnightMoves,
    BishopMoves,
    RookMoves,
    KingMoves,
    QueenMoves
};

int PIECE_MOVE_COUNTS[] = {PAWN_POSSIBLE_MOVES, KNIGHT_POSSIBLE_MOVES, BISHOP_POSSIBLE_MOVES, ROOK_POSSIBLE_MOVES, KING_POSSIBLE_MOVES, QUEEN_POSSIBLE_MOVES};


void StandardChessGame::reset() {
    GameOver = false;
    initTurn();
    initGame();
}

void StandardChessGame::initTurn() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, 1);
    currentTurn = (static_cast<Player>(distribution(gen) + 1));
}

void StandardChessGame::initGame(){
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++) {
            GamePiece pieceToPut = OPEN;
            Owner playerOwnerToPut = NONE;

            if (row < Y2) playerOwnerToPut = PTWO;
            if (row > Y5) playerOwnerToPut = PONE;

            if (row == Y0 || row == Y7) {
                switch (col) {
                    case X0:
                    case X7:
                        pieceToPut = ROOK;
                        break;
                    case X1:
                    case X6:
                        pieceToPut = KNIGHT;
                        break;
                    case X2:
                    case X5:
                        pieceToPut = BISHOP;
                        break;
                    case X3:
                        pieceToPut = KING;
                        break;
                    case X4:
                        pieceToPut = QUEEN;
                        break;
                }
            }

            if (row == Y1 || row == Y6) 
                pieceToPut = PAWN;

            GameBoard[row][col] = GameSquare(playerOwnerToPut, pieceToPut, Point(col, row));

            // if (pieceToPut == KING) {
            //     if (row == Y0)
            //         blackPlayerKing = &GameBoard[row][col];
            //     else
            //         whitePlayerKing = &GameBoard[row][col];
            // }

            if (pieceToPut == OPEN) // This first move made only applies to pieces with actual pieces there
                GameBoard[row][col].setFirstMoveMade();
        }
    }
}

bool StandardChessGame::onBoard(Point& p) {
    return (p.m_x <= 7 && p.m_x >= 0 && p.m_y <= 7 && p.m_y >= 0);
}

// True - valid moveset
// False - invalid moveset
bool StandardChessGame::validateMoveset(Move& move){

    GamePiece fromPiece = move.getMoveFrom().getPiece();
    short possibleMoveCounter = PIECE_MOVE_COUNTS[fromPiece - 1];

    if (fromPiece == PAWN && currentTurn == PlayerTwo)
        fromPiece = OPEN; 

    for(int move_set_count = 0; move_set_count < possibleMoveCounter; move_set_count++){

        // Point(move.getMoveFrom().getPosition() + Point(pieceMovePtrs[fromPiece][move_set_count][0], pieceMovePtrs[fromPiece][move_set_count][1])).print(); std::wcout << " --> "; move.getMoveTo().getPosition().print();
        // std::wcout << std::endl;
        if (move.getMoveFrom().getPosition() + Point(pieceMovePtrs[fromPiece][move_set_count][0], pieceMovePtrs[fromPiece][move_set_count][1]) == move.getMoveTo().getPosition()) {
            return true;
        }
    }

    return false;
}

// True - There is at least one move
// False - No moves from this piece
bool StandardChessGame::populatePossibleMoves(GameSquare& moveFrom) {

    GamePiece fromPiece = moveFrom.getPiece();
    short possibleMoveCounter = PIECE_MOVE_COUNTS[fromPiece - 1];

    if (fromPiece == PAWN && currentTurn == PlayerTwo)
        fromPiece = OPEN; 

    for(int move_set_count = 0; move_set_count < possibleMoveCounter; move_set_count++){

        // Iterating over entire moveset of a piece to see if it is 

        // 1. on the board
        // 2. unobstructed path
        // 3. piece at square owner is not equal to current turn (except speical moves!)
        // 3. if its a king then making sure nothing can reach that square

        Point pTemp(moveFrom.getPosition().m_x + pieceMovePtrs[fromPiece][move_set_count][0], moveFrom.getPosition().m_y + pieceMovePtrs[fromPiece][move_set_count][1]);
        
        if (!onBoard(pTemp))
            continue;

        Move mTemp(moveFrom, GameBoard[pTemp.m_y][pTemp.m_x]);

        if (!currTurnInCheck) {
            if (mTemp.getMoveTo().getPiece() != KING)
                if (unobstructedPathCheck(mTemp)) 
                    if (static_cast<Player>(mTemp.getMoveTo().getOwner()) != currentTurn) {
                        if (mTemp.getMoveFrom().getPiece() == KING) 
                            if (!kingSafeAfterMove(mTemp.getMoveTo()))
                                continue;
                        possibleMoves.push_back(possibleMoveType(&mTemp.getMoveTo(), 
                        mTemp.getMoveTo().getPiece() == GamePiece::OPEN ? possibleMoveTypes::POSSIBLE_MOVE_OPEN_SQAURE : possibleMoveTypes::POSSIBLE_MOVE_ENEMY_PIECE));
                    }
        } else {
            // See if making this move would make the kingSafe()
            bool isKingMove = moveFrom.getPiece() == KING;
            GameSquare saveOldFrom(mTemp.getMoveFrom());
            GameSquare saveOldTo(mTemp.getMoveTo());

            mTemp.getMoveTo().setPiece(mTemp.getMoveFrom().getPiece());
            mTemp.getMoveTo().setOwner(mTemp.getMoveFrom().getOwner());
            mTemp.getMoveFrom().setPiece(OPEN);
            mTemp.getMoveFrom().setOwner(NONE);

            // Making king move temporarily
            if (isKingMove) {
                if (currentTurn == PlayerOne) {
                    whitePlayerKing = &mTemp.getMoveTo();
                } else {
                    blackPlayerKing = &mTemp.getMoveTo();
                }
            }

            if(kingSafe())
                possibleMoves.push_back(possibleMoveType(&mTemp.getMoveTo(),
                isKingMove ? possibleMoveTypes::POSSIBLE_MOVE_KING_IN_DANGER :
                mTemp.getMoveTo().getPiece() == GamePiece::OPEN ? possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_SQUARE : possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_PIECE));
            
            // Revert
            mTemp.getMoveFrom().setPiece(saveOldFrom.getPiece());
            mTemp.getMoveFrom().setOwner(saveOldFrom.getOwner());
            mTemp.getMoveTo().setPiece(saveOldTo.getPiece());
            mTemp.getMoveTo().setOwner(saveOldTo.getOwner());

            // Revert king pos
            if (isKingMove) {
                if (currentTurn == PlayerOne) {
                    whitePlayerKing = &mTemp.getMoveFrom();
                } else {
                    blackPlayerKing = &mTemp.getMoveFrom();
                }       
            }
        }
    }
    return !possibleMoves.empty();
}

// True found matching move with possibleMoves
// False not found
possibleMoveTypes StandardChessGame::readPossibleMoves(GameSquare& to) {
    for(possibleMoveType& possibleMove: possibleMoves)
        if(to == *possibleMove.m_boardSquare)
            return possibleMove.possibleMoveTypeSelector;
    return possibleMoveTypes::NOT_FOUND;
}

// -1 Puts king in harm way
// 0 Invalid move
// 1 Piece taken
// 2 Piece moved
int StandardChessGame::makeMove(Move&& move){
    
    if(!readPossibleMoves(move.getMoveTo())) 
        return makeMove::INVALID_MOVE;

    // Check if making this move will put their king in check

    bool pieceTake = false;
    bool isKingMove = move.getMoveFrom().getPiece() == KING ? true : false;
    GameSquare saveOldFrom(move.getMoveFrom());
    GameSquare saveOldTo(move.getMoveTo());

    if(move.getMoveTo().getPiece() != OPEN)
        pieceTake = true;

    // Lets move the piece now, This is also where we would do something different in case of castling since you are not setting the from piece to none / open
    move.getMoveTo().setPiece(move.getMoveFrom().getPiece());
    move.getMoveTo().setOwner(move.getMoveFrom().getOwner());
    move.getMoveFrom().setPiece(OPEN);
    move.getMoveFrom().setOwner(NONE);

    if (isKingMove) {
        if (currentTurn == PlayerOne) {
            whitePlayerKing = &move.getMoveTo();
        } else {
            blackPlayerKing = &move.getMoveTo();
        }
    }

    if(kingSafe()){
        // Mark this gamesquare that a move has been made on this square
        move.getMoveFrom().setFirstMoveMade();

        if(pieceTake)
            return makeMove::PIECE_TAKEN;
        else 
            return makeMove::PIECE_MOVED;
    }

    // Revert move because this made the current turns king not safe
    move.getMoveFrom().setPiece(saveOldFrom.getPiece());
    move.getMoveFrom().setOwner(saveOldFrom.getOwner());
    move.getMoveTo().setPiece(saveOldTo.getPiece());
    move.getMoveTo().setOwner(saveOldTo.getOwner());

    // Revert king pos
    if (isKingMove) {
        if (currentTurn == PlayerOne) {
            whitePlayerKing = &move.getMoveFrom();
        } else {
            blackPlayerKing = &move.getMoveFrom();
        }
    }

    return makeMove::KING_IN_HARM;
}

// No Piece Present - 0
// This Piece Does not belong to you - 1
// Cannot take your own piece - 2
// Valid - 3
int StandardChessGame::validateGameSquare(GameSquare& square, enum getMoveType getMoveType){
    if (getMoveType == getMoveType::GET_FROM) {

        if(square.getOwner() == NONE)
            return validateGameSquare::NO_PIECE;

        if(static_cast<Player>(square.getOwner()) != currentTurn)
            return validateGameSquare::PIECE_NOT_YOURS;

    } else {
        if(static_cast<Player>(square.getOwner()) == currentTurn)
            return validateGameSquare::CANNOT_TAKE_OWN;
    }
    
    return validateGameSquare::VALID;
}

void StandardChessGame::printBoard(Player playerSideToPrint){
    if (GameOptions.clearScreenOnBoardPrint)
        system("clear");
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - row << " ";
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            std::wcout << "| ";
            wchar_t piece;


            if ((GameConnectivity == ONLINE_CONNECTIVITY && playerSideToPrint == PlayerTwo) || (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && playerSideToPrint == PlayerTwo)) {
                if(GameBoard[7 - row][7 - col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[7 - row][7 - col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }
            } else {
                if(GameBoard[row][col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[row][col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }
            }
            
            // if (GameOptions.flipBoardOnNewTurn && currentTurn == PlayerTwo) {
            //     if(GameBoard[7 - row][7 - col].getOwner() == NONE)
            //         piece = ' ';
            //     else if(GameBoard[7 - row][7 - col].getOwner() == PONE){
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
            //         set_terminal_color(GameOptions.p1_color);
            //     }else{
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
            //         set_terminal_color(GameOptions.p2_color);
            //     }
            // } else {
            //     if(GameBoard[row][col].getOwner() == NONE)
            //         piece = ' ';
            //     else if(GameBoard[row][col].getOwner() == PONE){
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[row][col].getPiece()];
            //         set_terminal_color(GameOptions.p1_color);
            //     }else{
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[row][col].getPiece()];
            //         set_terminal_color(GameOptions.p2_color);
            //     }
            // }
            
            std::wcout << piece;
            set_terminal_color(DEFAULT);
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
}

void StandardChessGame::printBoardWithMoves(Player playerSideToPrint) {
    if(GameOptions.clearScreenOnBoardPrint)
        system("clear");
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - row << " ";
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){

            std::wcout << "| ";
            wchar_t piece;
            possibleMoveTypes possibleMoveTypeSelector;

            if ((GameConnectivity == ONLINE_CONNECTIVITY && playerSideToPrint == PlayerTwo) || (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && playerSideToPrint == PlayerTwo)) {
                  if(GameBoard[7 - row][7 - col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[7 - row][7 - col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }

                // checking if the current square can be acctacked by piece
                possibleMoveTypeSelector = readPossibleMoves(GameBoard[7 - row][7 - col]);
                
            } else {
                if(GameBoard[row][col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[row][col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }

                // checking if the current square can be acctacked by piece
                possibleMoveTypeSelector = readPossibleMoves(GameBoard[row][col]);
            }

            if(possibleMoveTypeSelector != possibleMoveTypes::NOT_FOUND){
                if(piece == ' ')
                    piece = 'X';
                WRITE_COLOR color;
                switch(possibleMoveTypeSelector) {
                    case possibleMoveTypes::POSSIBLE_MOVE_ENEMY_PIECE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_OPEN_SQAURE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_PIECE:
                        color = WRITE_COLOR::BRIGHT_MAGENTA;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_SQUARE:
                        color = WRITE_COLOR::BRIGHT_YELLOW;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_KING_IN_DANGER:
                        color = WRITE_COLOR::YELLOW;
                        break;
                    default:
                        color = WRITE_COLOR::BLACK;
                        break;
                }
                set_terminal_color(color);
            }

            // Checking for highlighted piece, if flip board option is on then we need to look at the board the opposite way
            // otherwise if flip board is not active then we can always read it 0 - 7, 0 - 7
            if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == PlayerOne && fromHighlightedPiece == &GameBoard[row][col]) ||
                currentTurn == PlayerTwo && fromHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
                !GameOptions.flipBoardOnNewTurn && (fromHighlightedPiece == &GameBoard[row][col]))
                set_terminal_color(GameOptions.movingPiece_color);

            // Exact same thing as above but excpet now checking for toHighlightedPiece
            else if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == PlayerOne && toHighlightedPiece == &GameBoard[row][col]) ||
                currentTurn == PlayerTwo && toHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
                !GameOptions.flipBoardOnNewTurn && (toHighlightedPiece == &GameBoard[row][col]))
                set_terminal_color(GameOptions.movingToPiece_color);
            
            std::wcout << piece;    
            set_terminal_color(DEFAULT);
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
}

int StandardChessGame::reflectAxis(int val) {
    switch (val) {
        case 7:
            return 0;
        case 6:
            return 1;
        case 5:
            return 2;
        case 4:
            return 3;
        case 3:
            return 4;
        case 2:
            return 5;
        case 1:
            return 6;
        case 0:
            return 7;
        default:
            return -1;
    }
}

// -1 invalid input
// 0 options
// 1 valid input
int StandardChessGame::sanitizeGetMove(std::wstring& input) {

    if(!std::iswalpha(input[0])) 
        return -1; // Ensure [0] is alphabetical character
    
    // Force to lower case
    input[0] = std::towlower(input[0]); // Force [0] to lower case
    switch(input[0]) {
        case L'a': case L'b': case L'c': case L'd':
        case L'e': case L'f': case L'g': case L'h':
            break;
        case L'q': // Valid too but for option
        case L'x': case L'o':
            return 0;
        default: // Ensure one of the characters above
            return -1; // Ensure [0] is alphabetical character
    }

    // Make sure length is 2
    if(input.length() != 2)
        return -1; // Move must be length of 2

    // Ensure 2nd char is a number
    if(!std::iswdigit(input[1]) || input[1] == L'0' || input[1] == L'9') // Using std::iswdigit
        return -1; // Ensure [1] must be a digit and not '0' or '9'

    // Now we know it must be [0] == a-h, [1] == 1 - 8
    return 1;
} 

std::wstring StandardChessGame::playerToString(Player p){
    return (p == PlayerOne ? L"Player One" : L"Player Two");
}

GameSquare& StandardChessGame::convertMove(std::wstring move, Player sideToConvert){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top

    int row = 8 - (move[1] - 48);
    int col = move[0] - 97;

    if ((GameConnectivity == ONLINE_CONNECTIVITY && sideToConvert == PlayerTwo) ||
        (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && sideToConvert == PlayerTwo))
            return GameBoard[reflectAxis(row)][reflectAxis(col)];
        
    return GameBoard[row][col];

}

// 0 FREE
// 1 PONE TAKEN
// 2 PTWO TAKEN
enum Owner StandardChessGame::piecePresent(Point p){
    enum Owner owner = GameBoard[p.m_y][p.m_x].getOwner();
    if(owner == NONE && GameBoard[p.m_y][p.m_x].getPiece() == OPEN)
        return NONE;
    return owner;    
}

// True valid move
// False invalid move
bool StandardChessGame::verifyMove(Move& move){
    return validateMoveset(move) && unobstructedPathCheck(move);
}

bool StandardChessGame::verifyMove(Move&& move) { 
    return verifyMove(move);
}

// True - Good, clear path
// False - Something blocking
bool StandardChessGame::rookClearPath(Move& move){

    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    // Determine if moving along x or y axis
    if(from.m_x == to.m_x){
        // moving along Y axis
        int amount_to_check = std::abs(from.m_y - to.m_y);
        int direction = (from.m_y - to.m_y < 0) ? 1 : -1;

        for(int i = 1; i < amount_to_check; i++){
            struct Point temp = from;
            temp.m_y += (i * direction);
            if(piecePresent({temp.m_x, temp.m_y}))
                return false; 
        }
    }else{
        // moving along X axis
        int amount_to_check = std::abs(from.m_x - to.m_x);
        int direction = (from.m_x - to.m_x < 0) ? 1 : -1;

        for(int i = 1; i < amount_to_check; i++){
            struct Point temp = from;
            temp.m_x += (i * direction);
            if(piecePresent({temp.m_x, temp.m_y}))
                return false; 
        }
    }
    return true;
}

// True - Good, clear path
// False - Something blocking
bool StandardChessGame::bishopClearPath(Move& move){
    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    int xdir = (from.m_x - to.m_x < 0 ? 1 : -1);
    int ydir = (from.m_y - to.m_y < 0 ? 1 : -1);
    int amount_of_check = std::abs(from.m_x - to.m_x); // i dont think this should matter which one you do
    
    for(int i = 1; i < amount_of_check; i++){
        struct Point temp = from;
        temp.m_x += (i * xdir);
        temp.m_y += (i * ydir);
        if(piecePresent({temp.m_x, temp.m_y}))
            return false;
    }
    return true; 
}

// True - Valid move for pawn
// False - Invalid move for pawn
bool StandardChessGame::pawnMoveCheck(Move& move){

    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();
    
    // Validates double advance and diagonal only capturing

    // Rules of double advanwcing for pawns
    // 1. Starting Position
    // 2. Unobstructed Path
    // 3. No Capturing

    if(std::abs(from.m_y - to.m_y) == 2){

        // Check rule 2.
        if(std::abs(move.getMoveFrom().getPosition().m_y - move.getMoveTo().getPosition().m_y) == 2)
            if(piecePresent({from.m_x, from.m_y + (2 * (currentTurn == PlayerOne ? -1 : 1))}))
                return false;

        // Checking rule 1. and 3.
        if(move.getMoveFrom().getIfFirstMoveMade() || piecePresent({to.m_x, to.m_y}))
            return false;

    }else{
        // Now this section if for checking that pawn can only take an oppenents piece if moving diagonally

        if(from.m_x == to.m_x){ 
            // moving forward 1, meaning it cant take pieces
            if(piecePresent({to.m_x, to.m_y}))
                return false;
        }else{
            // moving diagonally, meaning it has to take a piece to do this
            if(!piecePresent({to.m_x, to.m_y}))
                return false;
        }
    }

    return true; // Meaning this is a valid move
}


// True - All good
// False - Piece in way
bool StandardChessGame::unobstructedPathCheck(Move& move){
    // Now verify if there is anything in path
    // Dont have to check knight bc it can go through pieces, also king can only go one piece

    switch(move.getMoveFrom().getPiece()){
        case(ROOK):
            return rookClearPath(move);
        case(QUEEN):
            if(move.getMoveFrom().getPosition().m_x == move.getMoveTo().getPosition().m_x || move.getMoveFrom().getPosition().m_y== move.getMoveTo().getPosition().m_y)
                return rookClearPath(move);
            else
                return bishopClearPath(move);
        case(BISHOP):
            return bishopClearPath(move);
        case(PAWN): 
            return pawnMoveCheck(move);
        default:
            return true; // Doesnt matter for (KNIGHT, KING & OPEN/NONE)
    }   
}

// True - can defend the king so not checkmate
// False - checkmate!

// if it is a queen causing check, called will temporarily change piece causing check to rook, then change it to bishop then change it back
GameSquare* StandardChessGame::canDefendKing(std::vector<GameSquare*>& teamPieces) {
    
    Point pieceCausingCheckPos = pieceCausingKingCheck->getPosition();
    GamePiece pieceCausingCheckPiece = pieceCausingKingCheck->getPiece();
    Point checkedKingPos = (currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing)->getPosition();

    int xdir = 0;
    int ydir = 0;
    int amount_to_check;

    if (pieceCausingCheckPiece == ROOK) {
            // Moving along Y axis    
        if (pieceCausingCheckPos.m_x == checkedKingPos.m_x)
            ydir = (pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0) ? 1 : -1;
        else// Moving along X axis
            ydir = (pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0) ? 1 : -1;
        // Get amount
        amount_to_check = xdir == 0 ? std::abs(checkedKingPos.m_y - pieceCausingCheckPos.m_y) : std::abs(checkedKingPos.m_x - pieceCausingCheckPos.m_x);
    } else if (pieceCausingCheckPiece == BISHOP) {
        xdir = pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0 ? -1 : 1;
        ydir = pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0 ? -1 : 1;
        amount_to_check = std::abs(pieceCausingCheckPos.m_x - checkedKingPos.m_x); 
    }

    if (pieceCausingCheckPiece == ROOK || pieceCausingCheckPiece == BISHOP) {
        for(int i = 1; i <= amount_to_check; i++){

            // Iterate over teamPieces to see if they can reach the currentSquare
            for(GameSquare* teamPiece: teamPieces){
                if(verifyMove(Move(*teamPiece, GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x]))) {
                    return &GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x]; // Some team piece can take or block the enemy piece that is causing the check on the king
                }
            }

            pieceCausingCheckPos.m_x += (1 * xdir);
            pieceCausingCheckPos.m_y += (1 * ydir);
        }
    } else {
        for(auto teamPiece: teamPieces){
            if(verifyMove(Move(*teamPiece, *pieceCausingKingCheck)))
                return pieceCausingKingCheck; // Some team piece can attack the knight, no gameover
        }
    } 

    return nullptr; // Meaning we could not find a move to make to save the king
}

// If nullptr is passed then it will check for general king saftey against every enemy piece
// If a pointer is passed it will check for saftey against the gamesquare passed
// True - King safe
// False - King NOT safe
bool StandardChessGame::kingSafe() {

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {

            GameSquare& curr = GameBoard[row][col];

            // If empty square we dont need to check, if current square is owned by 
            // current trun then we dont have to check it 
            if (curr.getOwner() == NONE || static_cast<Player>(curr.getOwner()) == currentTurn)
                continue;

            if (verifyMove(Move(curr, *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing)))) {
                pieceCausingKingCheck = &curr;
                return false;
            }
        }
    }

    return true;
}

bool StandardChessGame::kingSafeAfterMove(GameSquare& to) {
    
    GameSquare& currKing = *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing);
    currKing.setOwner(NONE);
    currKing.setPiece(OPEN);

    bool res = true;

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {

            GameSquare& curr = GameBoard[row][col];

            if (curr.getOwner() == NONE || static_cast<Player>(curr.getOwner()) == currentTurn)
                continue;

            if (verifyMove(Move(curr, to))) {
                res = false;
                goto restore_king;
            }
        }
    }
restore_king:
    currKing.setOwner(static_cast<Owner>(currentTurn));
    currKing.setPiece(KING);

    return res;
}

// True Gameover, Current turn loses
// False the king can get out of check
bool StandardChessGame::checkMate(){ // Checking everything around the king

    GameSquare& kingToCheckSafteyFor = *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing);
    short kingPossibleMoves = KING_POSSIBLE_MOVES; // 8
    std::vector<GameSquare*> teamPieces;
    std::vector<GameSquare*> enemyPieces;

    // Getting enemy and team pieces
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            GameSquare& gTemp = GameBoard[row][col];
            if(gTemp.getOwner() == NONE)
                continue;

            if(gTemp.getPiece() == KING)
                continue;

            else if(static_cast<Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
            else
                enemyPieces.push_back(&gTemp);
        }
    }
    
    // Check each square around the King, if its not on the board SKIP it
    // If its taken by a teammate then SKIP it, bc the king cant take own piece
    //! If its taken by an oppnent then we need to check if its a pawn OR knight

    // Since we are checking these pieces assuming the king is moving there we need to also assume the king has moved
    kingToCheckSafteyFor.setOwner(NONE);
    kingToCheckSafteyFor.setPiece(OPEN);

    bool res = true;

    for(int move_set_count = 0; move_set_count < kingPossibleMoves; move_set_count++){

        Point currKingPosAroundKing(kingToCheckSafteyFor.getPosition().m_x + pieceMovePtrs[KING][move_set_count][0], kingToCheckSafteyFor.getPosition().m_y + pieceMovePtrs[KING][move_set_count][1]);

        if(!onBoard(currKingPosAroundKing)) 
            continue; 

        // ! You would need to check here if the king can castle to get out of danger
        if(static_cast<Player>(GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x].getOwner()) == currentTurn)
            continue; 

        GameSquare& currSquareAroundKingCheck = GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x];
        bool ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = false;

        for(auto enemy: enemyPieces){

            if(verifyMove(Move(*enemy, currSquareAroundKingCheck))) {
                ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = true;
                break; // No point to keep checking, we know this square is NOT safe
            }
        }

        if (!ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE) {
            res = false;
            goto restore_king;
        }
    }

restore_king:

    kingToCheckSafteyFor.setOwner(static_cast<Owner>(currentTurn));
    kingToCheckSafteyFor.setPiece(KING);

    if(!res)
        return false;

    kingCanMakeMove = false;

    // Check if any of the teampieces can defend the king by either taking the piece causing check OR blocking its path
    
    if (pieceCausingKingCheck->getPiece() == QUEEN) {
        
        // pretend it is a rook temporarily
        pieceCausingKingCheck->setPiece(ROOK);

        if (canDefendKing(teamPieces)) {
            pieceCausingKingCheck->setPiece(QUEEN);
            return false;
        }

        // pretend it is bishop temporarily
        pieceCausingKingCheck->setPiece(ROOK);

        bool res = canDefendKing(teamPieces);

        pieceCausingKingCheck->setPiece(QUEEN);

        return res;
    }

    return canDefendKing(teamPieces) == nullptr; // If any of the team pieces can defend the king then this will result in NO checkmate, otherwise checkmate
}

