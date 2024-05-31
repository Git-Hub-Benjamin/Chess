#include "chess.hpp"
#include "text-piece-art.hpp"

static short Pawn1Moves[PAWN_POSSIBLE_MOVES][2] = { {0, -1}, {1, -1}, {-1, -1}, {0, -2} };
static short Pawn2Moves[PAWN_POSSIBLE_MOVES][2] = { {0, 1},  {-1, 1}, {1, 1}  , {0, 2}  };
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
    Pawn2Moves,
    Pawn1Moves,
    KnightMoves,
    BishopMoves,
    RookMoves,
    KingMoves,
    QueenMoves
};

int PIECE_MOVE_COUNTS[] = {PAWN_POSSIBLE_MOVES, KNIGHT_POSSIBLE_MOVES, BISHOP_POSSIBLE_MOVES, ROOK_POSSIBLE_MOVES, KING_POSSIBLE_MOVES, QUEEN_POSSIBLE_MOVES};

Standard_ChessGame::Standard_ChessGame(){
    initGame();
    GameOver = false;
}

Standard_ChessGame::Standard_ChessGame(bool dev_mode){
    DEV_MODE_ENABLE = dev_mode;
    GameOver = false;
    initGame();
}

void Standard_ChessGame::initGame(){
    memset(&GameBoard, 0, sizeof(GameBoard));
    currentTurn = PlayerOne;

    if(this->DEV_MODE_ENABLE){
        this->DEV_MODE_PRESET();
    }else{

        for(int row = 0; row < CHESS_BOARD_HEIGHT; row++)
        {
            for(int col = 0; col < CHESS_BOARD_WIDTH; col++)
            {
                GamePiece pieceToPut = OPEN;
                enum Owner playerOwnerToPut = NONE;
                if(row < Y2) playerOwnerToPut = PTWO;
                if(row > Y5) playerOwnerToPut = PONE;
                if(row == Y0 || row == Y7){

                    switch(col){
                        case X0:
                            pieceToPut = ROOK;
                            break;
                        case X1:
                            pieceToPut = KNIGHT;
                            break;
                        case X2:
                            pieceToPut = BISHOP;
                            break;
                        case X3:
                            pieceToPut = KING;
                            break;
                        case X4:
                            pieceToPut = QUEEN;
                            break;
                        case X5:
                            pieceToPut = BISHOP;
                            break;
                        case X6:
                            pieceToPut = KNIGHT;
                            break;
                        case X7:
                            pieceToPut = ROOK;
                            break;
                    }
                }
                if(row == Y1 || row == Y6) 
                    pieceToPut = PAWN;

                GameBoard[row][col] = GameSquare(playerOwnerToPut, pieceToPut, {col, row});

                if(pieceToPut == KING)
                    KingPositions[row == Y0 ? 1 : 0] = &GameBoard[row][col]; // if Y0 then it is player2 King, it not then its player1 king
                
                if(pieceToPut == OPEN) // This first move made only applies to pieces with actual pieces there
                    GameBoard[row][col].setFirstMoveMade();
            }
        }
    }
}

std::wstring Standard_ChessGame::playerToString(Player p){
    return (p == PlayerOne ? L"Player One" : L"Player Two");
}

void Standard_ChessGame::printBoard(){
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - i << " ";
        for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

            std::wcout << "| ";
            wchar_t piece;
            if(GameBoard[i][j].getOwner() == NONE)
                piece = ' ';
            else if(GameBoard[i][j].getOwner() == PONE){
                piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[i][j].getPiece()];
                set_terminal_color(GameOptions.p1_color);
            }else{
                piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[i][j].getPiece()];
                set_terminal_color(GameOptions.p2_color);
            }
            
            // print section --> "| ♟ " ex.
            std::wcout << piece;
            set_terminal_color(DEFAULT);
            std::wcout << " ";
            
        }
        std::wcout << "| " << CHESS_BOARD_HEIGHT - i << std::endl;
        std::wcout << "\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t    a   b   c   d   e   f   g   h\n";
}

GameSquare& Standard_ChessGame::convertMove(std::wstring move){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top

    return GameBoard[8 - (move[1] - 48)][move[0] - 97];
}

int Standard_ChessGame::validateMove(Move& move){
    if(move.getMoveFrom().getOwner() == NONE)
        return 0; // Cannot move a gamesquare without an owner

    if(move.getMoveFrom().getOwner() != currentTurn)
        return 1; // Gamesquare does not belong to you

    if(move.getMoveTo().getOwner() == currentTurn)
        return 2; // Cannot take own piece, CHECK CASTLING!

    return 3; // Valid
}

bool Standard_ChessGame::validateMoveset(Move& move){

    GamePiece fromPiece = move.getMoveFrom().getPiece();
    short possibleMoveCounter = PIECE_MOVE_COUNTS[fromPiece];

    for(int move_set_count = 0; move_set_count < possibleMoveCounter; move_set_count++){
        if (move.getMoveFrom().getPosition() + Point(pieceMovePtrs[fromPiece][move_set_count][0], pieceMovePtrs[fromPiece][move_set_count][1]) == move.getMoveTo().getPosition()) {
            return true;
        }
    }

    return false;
}

// 0 FREE
// 1 PONE TAKEN
// 2 PTWO TAKEN
enum Owner Standard_ChessGame::piecePresent(Point p){
    enum Owner owner = GameBoard[p.m_y][p.m_x].getOwner();
    if(owner == NONE && GameBoard[p.m_y][p.m_x].getPiece() == OPEN)
        return NONE;
    return owner;    
}

// 0 Invalid move
// 1 Piece in way
// 2 Valid move
int Standard_ChessGame::verifyMove(Move& move){
    if(validateMoveset(move))
        if(unobstructedPathCheck(move))
            return 2;
        else 
            return 1;
    return 0;
}

// True - Good, clear path
// False - Something blocking
bool Standard_ChessGame::rookClearPath(Move& move){

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
bool Standard_ChessGame::bishopClearPath(Move& move){
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
bool Standard_ChessGame::pawnMoveCheck(Move& move){

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
bool Standard_ChessGame::unobstructedPathCheck(Move& move){
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

// True  -> nothing can stop this attack >:)
// False -> means that not checkmate since a piece can block OR take the bishop
bool Standard_ChessGame::checkBishopCausingCheck(std::vector<GameSquare*>& teamPieces){

    Point pieceCausingCheckPos = pieceCausingKingCheck->getPosition();

    // This is checking the squares between the king and the bishop, bishop included, we are seeing if any teamPieces can get to any of them
    int xdir = pieceCausingKingCheck->getPosition().m_x - KingPositions[currentTurn]->getPosition().m_x < 0 ? -1 : 1;
    int ydir = pieceCausingKingCheck->getPosition().m_y - KingPositions[currentTurn]->getPosition().m_y < 0 ? -1 : 1;
    // i dont think this should matter which one you do
    int amount_to_check = std::abs(pieceCausingKingCheck->getPosition().m_x - KingPositions[currentTurn]->getPosition().m_x); 
    
    for(int i = 0; i < amount_to_check; i++){
        // Iterate over teamPieces to see if they can reach the currentSquare
        for(GameSquare* teamPiece: teamPieces){
            Move move = Move(*teamPiece, GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x]);
            if(verifyMove(move))
                return false; // Some team piece can take or block the enemy piece that is causing the check on the king
        }

        pieceCausingCheckPos.m_x += (i * xdir);
        pieceCausingCheckPos.m_y += (i * ydir);
    }
    return true; 
}


int Standard_ChessGame::makeMove(Move& move){

    int res = verifyMove(move);
    if ( res < 2 )
        return res;

    // Check if making this move will put their king in check

    bool pieceTake = false;
    GameSquare saveOldFrom(move.getMoveFrom());
    GameSquare saveOldTo(move.getMoveTo());

    if(move.getMoveFrom().getPiece() == KING)
        KingPositions[currentTurn] = &move.getMoveTo();

    if(move.getMoveTo().getPiece() != OPEN)
        pieceTake = true;

    // Lets move the piece now, This is also where we would do something different in case of castling since you are not setting the from piece to none / open
    move.getMoveTo().setPiece(move.getMoveFrom().getPiece());
    move.getMoveTo().setOwner(move.getMoveFrom().getOwner());
    move.getMoveFrom().setPiece(OPEN);
    move.getMoveFrom().setOwner(NONE);

    if(kingSafe()){
        
        // Mark this gamesquare that a move has been made on this square
        move.getMoveFrom().setFirstMoveMade();

        if(pieceTake)
            return 3;
        else 
            return 4;
    }

    // Revert move because this made the current turns king not safe
    move.getMoveFrom().setPiece(saveOldFrom.getPiece());
    move.getMoveFrom().setOwner(saveOldFrom.getOwner());
    move.getMoveTo().setPiece(saveOldTo.getPiece());
    move.getMoveTo().setOwner(saveOldTo.getOwner());

    if(move.getMoveFrom().getPiece() == KING)
        KingPositions[currentTurn] = &move.getMoveFrom();

    return 2;
}


GetMove Standard_ChessGame::getMove(int which){

    std::wstring temp_dst;

    while(true){
        bool optionMenu = false;
        // get input
        if (which == 0)
            std::wcout << playerToString(this->currentTurn) << ", Move: ";
        else 
            std::wcout << playerToString(this->currentTurn) << ", To: ";

        std::wcin >> temp_dst;
        
        if(!std::isalpha(temp_dst[0])) 
            continue; //! make sure [0] is alphabetical character
        
        // force to lower
        temp_dst[0] = std::tolower(temp_dst[0]); //! force [0] to lower case
        switch(temp_dst[0]){
            case L'a':
            case L'b':
            case L'c':
            case L'd':
            case L'e':
            case L'f':
            case L'g':
            case L'h': //* ALL VALID
                break;
            case L'q': //* VALID too but for option
            case L'x':
            case L'o':
                optionMenu = true;
                break;
            default: //! make sure one of the character above
                continue; //! make sure [0] is alphabetical character

        }

        if(optionMenu){
            std::wcout << "\n1. Change Colors\n" << "2. Continue\n" << "3. Quit\n\n--> "; 
            std::wcin >> temp_dst;
            switch(temp_dst[0]){
                case L'1':
                    std::wcout << "Not implemented." << std::endl;
                    break;
                case L'2':
                    continue;
                    break;
                case L'3':
                case L'q':
                    return GetMove(0);
                default:
                    break;
            }
            continue;
        }

        // make sure length is 2
        if(temp_dst.length() != 2)
            continue; //! move must be length of 2

        // make sure 2 char is a number
        if(!std::isdigit(temp_dst[1]) || temp_dst[1] == L'0' || temp_dst[1] == L'9') // using std::isdigit
            continue; //! make sure [1] must be a digit and not '0' OR '9' 

        // Now we know it must be [0] == a-h, [1] == 1 - 8

        temp_dst.append(L"\0"); //! add null terminator in case we want to print
        return GetMove(temp_dst, 1);
    }
}

void Standard_ChessGame::startGame(){

    std::wstring toPrint;

    while (true) {

        GetMove moveFrom = getMove(FROM_MOVE);
        if(!moveFrom.res){
            // Quit
        }
        GetMove moveTo = getMove(TO_MOVE);
        if(!moveTo.res){
            // Quit
        }

        Move move(convertMove(moveFrom.mMove), convertMove(moveTo.mMove));
        int res = validateMove(move);
        if (res != 3 ) {
            if (res == 0) 
                std::wcout << "No piece present." << std::endl;
            else if ( res == 1)
                std::wcout << "Piece does not belong to you." << std::endl;
            else 
                std::wcout << "Cannot take your own piece." << std::endl;
            continue;
        }

        res = makeMove(move);
        if (res < 2 ) {
            if (res == 0)
                std::wcout << "Invalid move." << std::endl;
            else
                std::wcout << "Invalid move, this puts you in check" << std::endl;
            continue;
        }else if( res == 2)
            std::wcout << "Piece moved." << std::endl;
        else 
            std::wcout << "Piece taken." << std::endl;

        break;
    }

    // Swap turns
    currentTurn = currentTurn == PlayerOne ? PlayerTwo : PlayerOne;

}



