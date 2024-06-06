#include "../chess.hpp"
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

Standard_ChessGame::Standard_ChessGame()
    : blackPlayerKing(&GameBoard[0][3]),
      whitePlayerKing(&GameBoard[7][3])
{
    initGame();
    GameOver = false;
}


Standard_ChessGame::Standard_ChessGame(Options gOptions, bool dev_mode)
    : GameOptions(gOptions),
      DEV_MODE_ENABLE(dev_mode),
      GameOver(false),
      blackPlayerKing(&GameBoard[0][3]),
      whitePlayerKing(&GameBoard[7][3])
{
    initGame();
}

Standard_ChessGame::Standard_ChessGame(Options gOptions)
    : GameOptions(gOptions),
      GameOver(false),
      blackPlayerKing(&GameBoard[0][3]),
      whitePlayerKing(&GameBoard[7][3])
{
    initGame();
}


void Standard_ChessGame::reset() {
    GameOver = false;
    initGame();
}

void Standard_ChessGame::initGame(){
    currentTurn = PlayerOne;

    if (this->DEV_MODE_ENABLE) {
        this->DEV_MODE_PRESET();
    } else {
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
}

std::wstring Standard_ChessGame::playerToString(Player p){
    return (p == PlayerOne ? L"Player One" : L"Player Two");
}

bool Standard_ChessGame::onBoard(Point& p) {
    return (p.m_x <= 7 && p.m_x >= 0 && p.m_y <= 7 && p.m_y >= 0);
}

void Standard_ChessGame::printBoard(){
    // for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
    //     for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
    //         GameBoard[row][col].print();
    //     }
    // }

    // return;

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

    if (!toPrint.empty()) {
        std::wcout << toPrint << std::endl;
        toPrint.clear();
    }
}

void Standard_ChessGame::printBoardWithMoves(GetMove moveFrom) {
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - row << " ";
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){

            std::wcout << "| ";
            wchar_t piece;
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
            if(readPossibleMoves(GameBoard[row][col], false)){
                if(piece == ' ')
                    piece = 'X';
                set_terminal_color(RED);
                std::wcout << piece;    
                set_terminal_color(DEFAULT);
                std::wcout << " ";
            }else{
                
                std::wcout << piece; 
                set_terminal_color(DEFAULT);
                std::wcout << " ";
            }    
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

GameSquare& Standard_ChessGame::convertMove(std::wstring move){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top

    return GameBoard[8 - (move[1] - 48)][move[0] - 97];
}

// True - Valid move
// False - Invalid Move
bool Standard_ChessGame::validateGameSquare(GameSquare& square, int which){
    if (which == FROM_MOVE) {

        if(square.getOwner() == NONE)
            toPrint = L"No piece present.";

        if(static_cast<Player>(square.getOwner()) != currentTurn)
            toPrint = L"This piece does not belong to you.";

    } else 
        if(static_cast<Player>(square.getOwner()) == currentTurn)
            toPrint = L"Cannot take your own piece";
    
    if (toPrint.empty())
        return true;
    //std::wcout << "To print --> " << toPrint << std::endl;
    return false;
}

// True - valid moveset
// False - invalid moveset
bool Standard_ChessGame::validateMoveset(Move& move){

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

// 0 FREE
// 1 PONE TAKEN
// 2 PTWO TAKEN
enum Owner Standard_ChessGame::piecePresent(Point p){
    enum Owner owner = GameBoard[p.m_y][p.m_x].getOwner();
    if(owner == NONE && GameBoard[p.m_y][p.m_x].getPiece() == OPEN)
        return NONE;
    return owner;    
}

// True valid move
// False invalid move
bool Standard_ChessGame::verifyMove(Move& move){
    return validateMoveset(move) && unobstructedPathCheck(move);
}

bool Standard_ChessGame::verifyMove(Move&& move) { 
    return verifyMove(move);
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

// True - can defend the king so not checkmate
// False - checkmate!

// if it is a queen causing check, called will temporarily change piece causing check to rook, then change it to bishop then change it back
bool Standard_ChessGame::canDefendKing(std::vector<GameSquare*>& teamPieces) {
    
    Point pieceCausingCheckPos = pieceCausingKingCheck->getPosition();
    GamePiece pieceCausingCheckPiece = pieceCausingKingCheck->getPiece();
    Point checkedKingPos = (currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing)->getPosition();

    int xdir;
    int ydir;
    int amount_to_check;

    if (pieceCausingCheckPiece == ROOK) {
        xdir = (pieceCausingCheckPos.m_x == (currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing)->getPosition().m_x) ? (pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0 ? -1 : 1) : 0;
        ydir = xdir == 0 ? pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0 ? 1 : -1 : 0;
        amount_to_check = xdir == 0 ? std::abs(checkedKingPos.m_y - pieceCausingCheckPos.m_y) : std::abs(checkedKingPos.m_x - pieceCausingCheckPos.m_x);
    } else if (pieceCausingCheckPiece == BISHOP) {
        xdir = pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0 ? -1 : 1;
        ydir = pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0 ? -1 : 1;
        amount_to_check = std::abs(pieceCausingCheckPos.m_x - checkedKingPos.m_x); 
    }

    if (pieceCausingCheckPiece == ROOK || pieceCausingCheckPiece == BISHOP) {
        for(int i = 0; i < amount_to_check; i++){
            // Iterate over teamPieces to see if they can reach the currentSquare
            for(GameSquare* teamPiece: teamPieces){
                if(verifyMove(Move(*teamPiece, GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x])))
                    return false; // Some team piece can take or block the enemy piece that is causing the check on the king
            }

            pieceCausingCheckPos.m_x += (i * xdir);
            pieceCausingCheckPos.m_y += (i * ydir);
        }
    } else {
        for(auto teamPiece: teamPieces){
            if(verifyMove(Move(*teamPiece, *pieceCausingKingCheck)))
                return false; // Some team piece can attack the knight, no gameover
        }
    } 

    return true;
}


int Standard_ChessGame::makeMove(Move&& move){
    
    if (GameOptions.moveHighlighting) {
        if(!readPossibleMoves(move.getMoveTo(), true))
            return 0;
    } else {
        if(!verifyMove(move))
            return 0;
    }

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
            toPrint = L"Piece taken.";
        else 
            toPrint = L"Piece moved.";

        return true;
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

    toPrint = L"This puts your king in danger!";

    return false;
}

// If nullptr is passed then it will check for general king saftey against every enemy piece
// If a pointer is passed it will check for saftey against the gamesquare passed
// True - King safe
// False - King NOT safe
bool Standard_ChessGame::kingSafe() {

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

bool Standard_ChessGame::kingSafeAfterMove(GameSquare& to) {
    
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

bool Standard_ChessGame::checkMate(){ // Checking everything around the king

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
            else if(static_cast<Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
            else
                enemyPieces.push_back(&gTemp);
        }
    }
    
    // Check each square around the King, if its not on the board SKIP it
    // If its taken by a teammate then SKIP it, bc the king cant take own piece
    // If its taken by an oppnent then we need to check if its a pawn OR knight

    // Since we are checking these pieces assuming the king is moving there we need to also assume the king has moved
    kingToCheckSafteyFor.setOwner(NONE);
    kingToCheckSafteyFor.setPiece(OPEN);

    bool res = true;

    for(int move_set_count = 0; move_set_count < kingPossibleMoves; move_set_count++){

        Point currKingPosAroundKing(pieceMovePtrs[KING][move_set_count][0], pieceMovePtrs[KING][move_set_count][1]);
        GameSquare& currSquareAroundKingCheck = GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x];

        if(!onBoard(currKingPosAroundKing)) 
            continue; 

        // ! You would need to check here if the king can castle to get out of danger
        if(static_cast<Player>(currSquareAroundKingCheck.getOwner()) == currentTurn) 
            continue; 

        bool ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = false;
 
        for(auto enemy: enemyPieces){
            if(verifyMove(Move(*enemy, kingToCheckSafteyFor))) {
                ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = true;
                goto restore_king;
            }
        }
    }

restore_king:

    kingToCheckSafteyFor.setOwner(static_cast<Owner>(currentTurn));
    kingToCheckSafteyFor.setPiece(KING);

    if(!res)
        return false;
    

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

    return canDefendKing(teamPieces); // If any of the team pieces can defend the king then this will result in NO checkmate, otherwise checkmate
}

// True - There is at least one move
// False - No moves from this piece
bool Standard_ChessGame::populatePossibleMoves(GameSquare& moveFrom) {

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
        
        // std::wcout << "Moveset calculation: "; pTemp.print(); std::wcout << std::endl;

        Move mTemp(moveFrom, GameBoard[pTemp.m_y][pTemp.m_x]);

        // std::wcout << "Move, From: "; mTemp.getMoveFrom().print(); std::wcout << ", To: "; mTemp.getMoveTo().print(); std::wcout << std::endl;

        if (onBoard(pTemp)) 
            if (unobstructedPathCheck(mTemp)) 
                if (static_cast<Player>(mTemp.getMoveTo().getOwner()) != currentTurn) {
                    if (mTemp.getMoveFrom().getPiece() == KING && !kingSafeAfterMove(mTemp.getMoveTo())) 
                        continue;
                    possibleMoves.push_back(&mTemp.getMoveTo());
                }

    }

    if (possibleMoves.empty())
        toPrint = L"No moves with that piece.";

    return !possibleMoves.empty();
}

// True found matching move with possibleMoves
// False not found
bool Standard_ChessGame::readPossibleMoves(GameSquare& to, bool ADD_PRINT) {
    for(GameSquare* Square: possibleMoves) {
        if(to == *Square)
            return true;
    }
    if (ADD_PRINT)
        toPrint = L"Invalid move.";
    return false;
}


GetMove Standard_ChessGame::getMove(int which){

    std::wstring temp_dst;

    while (true){
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

    while (!GameOver) {

            std::wcout << "White Player King: "; whitePlayerKing->print(); std::wcout << std::endl;
            std::wcout << "Black Player King: "; blackPlayerKing->print(); std::wcout << std::endl;

            // Reset check
            currTurnInCheck = false;

            if (!kingSafe()) {
                // if (checkMate()) {

                // } 
                std::wcout << "In check!" << std::endl;
                currTurnInCheck = true;
            }

            while (true) {

                // Clear the possibleMoves vec
                possibleMoves.clear();
            
                GetMove moveFrom; 
                
                if (!currTurnInCheck) {

                    printBoard();

                    moveFrom = getMove(FROM_MOVE);
                    if(!moveFrom.res){
                        // Quit
                        GameOver = true;
                        break;
                    }

                    if (!validateGameSquare(convertMove(moveFrom.mMove), FROM_MOVE))
                        continue;
                }


                // Still going to populate this even if piece highlighting is not enabled so we can print when a piece has no valid moves
                if (!populatePossibleMoves(currTurnInCheck ? *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing) : convertMove(moveFrom.mMove)))
                    continue;

                if (GameOptions.moveHighlighting) {

                    printBoardWithMoves(moveFrom);
                
                // If no move highlighting, and in check then now we print board
                } else if (currTurnInCheck) 
                    printBoard();

                GetMove moveTo = getMove(TO_MOVE);
                if(!moveTo.res){
                    // Quit
                    GameOver = true;
                    break;
                }

                if (!validateGameSquare(convertMove(moveTo.mMove), TO_MOVE))
                    continue;


                if (!makeMove(currTurnInCheck ? Move(*(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing), convertMove(moveTo.mMove)) : Move(convertMove(moveFrom.mMove), convertMove(moveTo.mMove))))
                    continue;

                break;
            }

        // Swap turns
        currentTurn = currentTurn == PlayerOne ? PlayerTwo : PlayerOne;
    }

}



