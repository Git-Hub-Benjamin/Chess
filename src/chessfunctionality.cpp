#include "chess.h"
#include <vector>
#include <unordered_map>

// Conversion from string to string
std::string convertString(std::string &passed){
    return std::string(passed.begin(), passed.end());
}

// Global for the program

struct Piece_moveset Pawn1 = {nullptr};
struct Piece_moveset Pawn2 = {nullptr};
struct Piece_moveset Knight = {nullptr};
struct Piece_moveset Bishop = {nullptr};
struct Piece_moveset Rook = {nullptr};
struct Piece_moveset Queen = {nullptr};
struct Piece_moveset King = {nullptr};

int PIECE_MOVE_COUNTS[] = {PAWN_POSSIBLE_MOVES, KNIGHT_POSSIBLE_MOVES, BISHOP_POSSIBLE_MOVES, ROOK_POSSIBLE_MOVES, KING_POSSIBLE_MOVES, QUEEN_POSSIBLE_MOVES};

struct Piece_moveset* PIECE_MOVES[] = {&Pawn2, &Pawn1, &Knight, &Bishop, &Rook, &King, &Queen};

wchar_t piece_art_p1[7] = {' ', L'♟', L'♞', L'♝', L'♜', L'♚', L'♛'};
wchar_t piece_art_p2[7] = {' ', L'♙', L'♘', L'♗', L'♖', L'♔', L'♕'};

// wchar_t piece_art_p1[7] = {' ', L'P', L'K', L'B', L'R', L'K', L'Q'};
// wchar_t piece_art_p2[7] = {' ', L'p', L'k', L'b', L'r', L'k', L'q'};

// Class functions

ChessGame::ChessGame(bool dev_mode){
    DEV_MODE_ENABLE = dev_mode;
    gameover = false;
    init();
}

void ChessGame::reset(){
    init();
}

// 0 Good, 1 Bad
void ChessGame::init(){
    memset(&GameBoard, 0, sizeof(GameBoard));
    currentTurn = PONE;

    if(this->DEV_MODE_ENABLE){
        this->DEV_MODE_PRESET();
    }else{

        for(int row = 0; row < CHESS_BOARD_HEIGHT; row++)
        {
            for(int col = 0; col < CHESS_BOARD_WIDTH; col++)
            {
                enum GamePiece temp = OPEN;
                GameBoard[row][col].pos = {col, row};
                if(row < Y2) GameBoard[row][col].ownership = PTWO;
                if(row > Y5) GameBoard[row][col].ownership = PONE;
                if(row == Y0 || row == Y7){

                    switch(col){
                        case X0:
                            temp = ROOK;
                            break;
                        case X1:
                            temp = KNIGHT;
                            break;
                        case X2:
                            temp = BISHOP;
                            break;
                        case X3:
                            temp = KING;
                            break;
                        case X4:
                            temp = QUEEN;
                            break;
                        case X5:
                            temp = BISHOP;
                            break;
                        case X6:
                            temp = KNIGHT;
                            break;
                        case X7:
                            temp = ROOK;
                            break;
                    }

                    GameBoard[row][col].piece = temp;
                }
                if(row == Y1 || row == Y6) 
                    GameBoard[row][col].piece = PAWN;

                if(temp == KING){
                    KingPositions[row == Y0 ? 1 : 0] = &GameBoard[row][col]; // if Y0 then it is player2 King, it not then its player1 king
                }
            }
        }
    }
}

// Logic functions

// 0 FREE
// 1 PONE TAKEN
// 2 PTWO TAKEN
enum Owner piecePresent(ChessGame &game, struct Point p){
    enum Owner owner = game.GameBoard[p.y][p.x].ownership;
    if(owner == NONE && game.GameBoard[p.y][p.x].piece == OPEN)
        return NONE;
    return owner;
    
}

static bool pawnHelperDblAdvCapture(ChessGame &game, GameSqaure &pawnFrom, GameSqaure &to){
    
    // Rules of double advanwcing for pawns
    // 1. Starting Position
    // 2. Unobstructed Path
    // 3. No Capturing

    if(std::abs(pawnFrom.pos.y - to.pos.y) == 2){
        std::wcout << "Pawn double advanwcing" << std::endl; 
        if(!pawnClearPath(game, pawnFrom, to)) // checking 2.
            return false; 
        if(game.currentTurn == PONE){ // checking 1. and 3.
            if(pawnFrom.pos.y != 6 || piecePresent(game, {to.pos.x, to.pos.y}))
                return false;
        }else{
            if(pawnFrom.pos.y != 1 || piecePresent(game, {to.pos.x, to.pos.y}))
                return false;
        }
    }else{
        std::wcout << "Pawn moving 1 square" << std::endl;
        // Now this section if for checking that pawn can only take an oppenents piece if moving diagonally
        if(pawnFrom.pos.x == to.pos.x){ 
            std::wcout << "Moving straight" << std::endl;
            // moving forward 1, meaning it cant take pieces
            if(piecePresent(game, {to.pos.x, to.pos.y}))
                return false;
        }else{
            std::wcout << "Moving diagonally" << std::endl;
            // moving diagonally, meaning it has to take a piece to do this
            if(!piecePresent(game, {to.pos.x, to.pos.y}))
                return false;
        }
    }

    return true; // Meaning this is a valid move
}

// True means the path is clear, it can make this move
// False means there is something in the way
static bool unobstructed_path_check(ChessGame &game, GameSqaure& from, GameSqaure &to){
    // Now verify if there is anything in path
    // Dont have to check knight bc it can go through pieces, also king can only go one piece
    switch(from.piece){
        case(ROOK):
            return rookClearPath(game, from, to);
            break;
        case(QUEEN):
            if(rookClearPath(game, from, to))
                return bishopClearPath(game, from, to);
            break;
        case(BISHOP):
            return bishopClearPath(game, from, to);
            break;
        case(PAWN): 
            return pawnHelperDblAdvCapture(game, from, to);
            break;
        default:
            break;
    }   
    return true; // Doesnt matter for (KNIGHT, KING & OPEN/NONE)
}

// True  -> nothing can stop this attack >:)
// False -> means that not checkmate since a piece can block OR take the rook 
static bool rook_causing_check(ChessGame &game, GameSqaure& checkedKing, std::vector<GameSqaure*>& teamPieces){
    GameSqaure temp = game.pieceCausingKingCheck;
                                                                        // if on same x axis, then moving up or down, 
    int xdir = (game.pieceCausingKingCheck.pos.x == checkedKing.pos.x) ? (game.pieceCausingKingCheck.pos.x - checkedKing.pos.x < 0 ? -1 : 1) : 0;
                // if x == 0 then it must be on the same y axis value, so going left or right, 
    int ydir = xdir == 0 ? game.pieceCausingKingCheck.pos.y - checkedKing.pos.y < 0 ? 1 : -1 : 0;
    int amount_to_check = xdir == 0 ? std::abs(checkedKing.pos.y - game.pieceCausingKingCheck.pos.y) : std::abs(checkedKing.pos.x - game.pieceCausingKingCheck.pos.x);

    for(int i = 0; i < amount_to_check; i++){

        // Iterate over teamPieces to see if they can reach the currentSquare
        for(GameSqaure* teamPiece: teamPieces){
            if(verifyMove(game, *teamPiece, temp))
                return false; // Some team piece can take or block the enemy piece that is causing the check on the king
        }

        // Now we know this square didnt work, get the next one
        temp.pos.x += (i * xdir);
        temp.pos.y += (i * ydir);
    }

    return true;
}

// True  -> nothing can stop this attack >:)
// False -> means that not checkmate since a piece can block OR take the bishop
static bool bishop_causing_check(ChessGame &game, GameSqaure& checkedKing, std::vector<GameSqaure*>& teamPieces){
    // Up Or Down
    GameSqaure temp = game.pieceCausingKingCheck;

    int xdir = game.pieceCausingKingCheck.pos.x - checkedKing.pos.x < 0 ? -1 : 1;
    int ydir = game.pieceCausingKingCheck.pos.y - checkedKing.pos.y < 0 ? -1 : 1;
    int amount_to_check = std::abs(game.pieceCausingKingCheck.pos.x - checkedKing.pos.x); // i dont think this should matter which one you do
    
    for(int i = 0; i < amount_to_check; i++){
        // Iterate over teamPieces to see if they can reach the currentSquare
        for(GameSqaure* teamPiece: teamPieces){
            if(verifyMove(game, *teamPiece, temp))
                return false; // Some team piece can take or block the enemy piece that is causing the check on the king
        }

        temp.pos.x += (i * xdir);
        temp.pos.y += (i * ydir);
    }
    return true; 
}

bool onBoard(Point& p){
    return (p.x <= 7 && p.x >= 0 && p.y <= 7 && p.y >= 0);
}

void print_messages(std::deque<std::wstring> deque){
    for(auto it = deque.begin(); it != deque.end(); it++){
        std::wcout << *it << std::endl; // printt the curr msg
    }
    while (!deque.empty()) {
        deque.pop_front();
    }
}

static std::vector<GameSqaure*>* get_move_to_squares(ChessGame &game, GameSqaure& from){
    std::vector<GameSqaure*>* squares = new std::vector<GameSqaure*>;
    
    //* SETUP --> very similar to verifyMove function
                                                // -1  since we dont use OPEN (0)
    short possibleMoveCounter = PIECE_MOVE_COUNTS[from.piece - 1];
    enum GamePiece piece = from.piece;
    struct Piece_moveset* PIECE_MOVESET = PIECE_MOVES[from.piece];
    
    // if its a pawn, and its the player two turn then set this bc ptwo has
    // a different moveset for the pon
    if(game.currentTurn == PTWO && piece == PAWN) 
        PIECE_MOVESET = PIECE_MOVES[0];
    
    for(int i = 0; i < possibleMoveCounter; i++){
        // since we are testing this every time we need to make a copy everytime
        struct Point pointToMoveFrom = from.pos; // make a copy of from

        pointToMoveFrom.x += PIECE_MOVESET->moves[i].x;
        pointToMoveFrom.y += PIECE_MOVESET->moves[i].y;

        // If on board, nothing in the way, and not attacking own teammate, then add it to the vector
        if(onBoard(pointToMoveFrom) && 
            unobstructed_path_check(game, from, game.GameBoard[pointToMoveFrom.y][pointToMoveFrom.x]) && 
            game.GameBoard[pointToMoveFrom.y][pointToMoveFrom.x].ownership != game.currentTurn){
            squares->push_back(&game.GameBoard[pointToMoveFrom.y][pointToMoveFrom.x]);
        }
    }
    return squares; // If this returns with a size of 0, then that means this piece CANNOT make any moves 

}

static bool in_vec_squares(std::vector<GameSqaure*>* vec, GameSqaure& check) {
    if (vec == nullptr) {
        // Handle nullptr
        return false;
    }
    
    for (GameSqaure* square : *vec) { 
        if (square->pos.x == check.pos.x && square->pos.y == check.pos.y) {
            return true;
        }
    }
    return false;
}

void print_board_with_moves(ChessGame &game, GameSqaure& from){
    // first get the gameSquares that the piece can move to into a vector
    std::vector<GameSqaure*>* squaresPieceCanMoveTo = get_move_to_squares(game, from);

    std::wcout << "\t\t\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
        std::wcout << "\t\t\t\t\t" << CHESS_BOARD_HEIGHT - i << " ";
        for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

            wchar_t piece;
            if(game.GameBoard[i][j].ownership == NONE)
                piece = ' ';
            else if(game.GameBoard[i][j].ownership == PONE)
                piece = piece_art_p1[game.GameBoard[i][j].piece];
            else
                piece = piece_art_p2[game.GameBoard[i][j].piece];

            // checking if we need to change the color
            if(in_vec_squares(squaresPieceCanMoveTo, game.GameBoard[i][j])){
                std::wcout << "| ";
                if(piece == ' ')
                    piece = 'X';
                set_terminal_color(RED);
                std::wcout << piece;    
                set_terminal_color(BOLD);
                std::wcout << " ";
            }else
                std::wcout << "| " << piece << " ";
        }
        std::wcout << "| " << CHESS_BOARD_HEIGHT - i << std::endl;
        std::wcout << "\t\t\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t\t\t    a   b   c   d   e   f   g   h\n";
}

void print_board(ChessGame &game){    
    std::wcout << "\t\t\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
        std::wcout << "\t\t\t\t\t" << CHESS_BOARD_HEIGHT - i << " ";
        for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

            wchar_t piece;
            if(game.GameBoard[i][j].ownership == NONE)
                piece = ' ';
            else if(game.GameBoard[i][j].ownership == PONE)
                piece = piece_art_p1[game.GameBoard[i][j].piece];
            else
                piece = piece_art_p2[game.GameBoard[i][j].piece];

            
            // print section --> "| ♟ " ex.
            std::wcout << "| " << piece << " ";
        }
        std::wcout << "| " << CHESS_BOARD_HEIGHT - i << std::endl;
        std::wcout << "\t\t\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t\t\t    a   b   c   d   e   f   g   h\n";
}


//0; // Successful move
//1; // Successful move piece taken
//2; // Unsuccessful move
int makeMove(ChessGame &game, GameSqaure &from, GameSqaure &to){

    int res = 0;
    // If we are tracking taken pieces this is where we would do it
    if(to.ownership != NONE && to.piece != OPEN){
        res = 1;
    }
    
    if(from.piece == KING)
        game.KingPositions[game.currentTurn - 1] = &to; // if moving king, update the king position for the current player to the to gameSquare

    // Lets move the piece now
    game.GameBoard[to.pos.y][to.pos.x].piece = from.piece;
    game.GameBoard[to.pos.y][to.pos.x].ownership = from.ownership;
    game.GameBoard[from.pos.y][from.pos.x].piece = OPEN;
    game.GameBoard[from.pos.y][from.pos.x].ownership = NONE;

    
    return res;

}

void validateMovePiece(ChessGame& game, GameSqaure& movePiece, std::wstring& retMsg){
    retMsg = L"";
    // if there is no piece at the point trying to move from
    if(movePiece.piece == OPEN || movePiece.ownership == NONE){
        retMsg = L"No piece present."; //! cannot move a piece of OPEN, cannot move an open square
    }

    // make sure piece belongs to current turn
    if(movePiece.ownership != game.currentTurn){
        retMsg = L"Piece does not belong to you."; //! cannot move a square of the other players piece
    }
}

void validateMoveToPiece(ChessGame& game, GameSqaure& moveToSquare, std::wstring& retMsg){
    retMsg = L"";

    if(moveToSquare.ownership == game.currentTurn){
        retMsg = L"Cannot take own piece."; //! cannot take own piece
        // hanlde castling later, its complex...
    }
}

static bool rookClearPath(ChessGame &game, GameSqaure &from, GameSqaure &to){
    // Determine if moving along x or y axis
    if(from.pos.x == to.pos.x){
        // moving along Y axis
        int amount_to_check = std::abs(from.pos.y - to.pos.y);
        int direction = (from.pos.y - to.pos.y < 0) ? 1 : -1;

        for(int i = 1; i < amount_to_check; i++){
            struct Point temp = from.pos;
            temp.y += (i * direction);
            if(piecePresent(game, {temp.x, temp.y}))
                return false; 
        }
    }else{
        // moving along X axis
        int amount_to_check = std::abs(from.pos.x - to.pos.x);
        int direction = (from.pos.x - to.pos.x < 0) ? 1 : -1;

        for(int i = 1; i < amount_to_check; i++){
            struct Point temp = from.pos;
            temp.x += (i * direction);
            if(piecePresent(game, {temp.x, temp.y}))
                return false; 
        }
    }
    return true;
}

static bool bishopClearPath(ChessGame &game, GameSqaure &from, GameSqaure &to){
    int xdir = from.pos.x - to.pos.x < 0 ? 1 : -1;
    int ydir = from.pos.y - to.pos.y < 0 ? 1 : -1;
    int amount_of_check = std::abs(from.pos.x - to.pos.x); // i dont think this should matter which one you do
    
    for(int i = 1; i < amount_of_check; i++){
        struct Point temp = from.pos;
        temp.x += (i * xdir);
        temp.y += (i * ydir);
        if(piecePresent(game, {temp.x, temp.y}))
            return false;
    }
    return true; 
}

static bool pawnClearPath(ChessGame &game, GameSqaure &from, GameSqaure &to){
    if(std::abs(from.pos.y - to.pos.y) == 2){
        // check the 1 spot between
        int direction = game.currentTurn == PONE ? -1 : 1;
        if(piecePresent(game, {from.pos.x, from.pos.y + (2 * direction)}))
            return false; // if there is a piece present then return false
    }
    return true;
}

static bool validateMoveset(ChessGame &game, GameSqaure &from, GameSqaure &to){
    // we know from has a piece present
    //* SETUP
                                                // -1  since we dont use OPEN (0)
    short possibleMoveCounter = PIECE_MOVE_COUNTS[from.piece - 1];
    enum GamePiece piece = from.piece;
    struct Piece_moveset* PIECE_MOVESET = PIECE_MOVES[from.piece];
    
    // if its a pawn, and its the player two turn then set this bc ptwo has
    // a different moveset for the pon
    if(game.currentTurn == PTWO && piece == PAWN) 
        PIECE_MOVESET = PIECE_MOVES[0];

    struct Point pointToGetTo = to.pos; // make a copy of to
    
    for(int i = 0; i < possibleMoveCounter; i++){
        // since we are testing this every time we need to make a copy everytime
        struct Point pointToMoveFrom = from.pos; // make a copy of from

        pointToMoveFrom.x += PIECE_MOVESET->moves[i].x;
        pointToMoveFrom.y += PIECE_MOVESET->moves[i].y;

        if(pointToMoveFrom.x == pointToGetTo.x && pointToMoveFrom.y == pointToGetTo.y){
            return true; // Yes! the piece can reach this gameSquare
        }
    } 
    return false; // No, the piece cannot reach this gameSquare
}


//* use this at the beggining of a turn to make sure that the oppenent didnt put you in check, and use this after your piece moves to make sure
//* moving your piece did not put your king in check (kingSafe)

bool kingSafe(ChessGame& game){ // Just checking the king square
    GameSqaure& kingToCheckSafteyFor = *game.KingPositions[game.currentTurn - 1];

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            GameSqaure& currentSquare = game.GameBoard[row][col];
            // if its open or its the current players turn then we dont need to check against it
            if(currentSquare.ownership == NONE || currentSquare.piece == OPEN || currentSquare.ownership == game.currentTurn)
                continue;
            
            if(verifyMove(game, currentSquare, kingToCheckSafteyFor)){ 
                // if this returns true then it means that the currentSquare can actually attack the king, 
                // BUT this doesnt apply for pawns, we need to pawns we need to do extra check for them 
                
                game.pieceCausingKingCheck = currentSquare; // Needed for checkMate
                std::wcout << "Piece causing check: " << game.pieceCausingKingCheck.piece << ", Pos: {" << game.pieceCausingKingCheck.pos.x << ", " <<
                game.pieceCausingKingCheck.pos.y << "}" << std::endl; 
                return false; // KING IS NOT SAFE
            }
        }
    }
    return true; // KING IS SAFE
}

// This will be called if kingSafe returns false, meaning the king is in check, we need to see if there is somewhere the king can go to get out of it

bool checkMate(ChessGame &game){ // Checking everything around the king

    GameSqaure& kingToCheckSafteyFor = *game.KingPositions[game.currentTurn - 1];
    short kingPossibleMoves = KING_POSSIBLE_MOVES; // 8
    struct Piece_moveset* KING_MOVESET = PIECE_MOVES[KING];
    std::vector<GameSqaure*> teamPieces;
    std::vector<GameSqaure*> enemyPieces;

    // Getting enemy and team pieces
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            GameSqaure& currBoardSquare = game.GameBoard[row][col];
            if(currBoardSquare.ownership == NONE)
                continue;
            else if(currBoardSquare.ownership == game.currentTurn)
                teamPieces.push_back(&currBoardSquare);
            else
                enemyPieces.push_back(&currBoardSquare);
        }
    }
    
    // Check each square around the King, if its not on the board SKIP it
    // If its taken by a teammate then SKIP it, bc the king cant take own piece
    // If its taken by an oppnent then we need to check if its a pawn OR knight
    for(int i = 0; i < kingPossibleMoves; i++){

        struct Point currGamePosCheck = kingToCheckSafteyFor.pos;
        currGamePosCheck.x += KING_MOVESET->moves[i].x;
        currGamePosCheck.y += KING_MOVESET->moves[i].y;
        
        if(!onBoard(currGamePosCheck)) // Not on gameboard, king cant move here
            continue; 


        bool ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = false;
        GameSqaure& currSquareAroundKingCheck = game.GameBoard[currGamePosCheck.y][currGamePosCheck.x];

        if(currSquareAroundKingCheck.ownership == game.currentTurn) // Piece at position is taken by team piece, cannot move here
            continue; 

        for(GameSqaure* enemy: enemyPieces){
            if(verifyMove(game, *enemy, currSquareAroundKingCheck))
                ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = true;
        }

        if(!ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE)
            return false;
    }

    // Now we need to see if any of the team pieces can block the enemy piece that is causing the team king to be in check
    switch(game.pieceCausingKingCheck.piece){
        case(ROOK):
            return rook_causing_check(game, kingToCheckSafteyFor, teamPieces);
            break;
        case(QUEEN):
            if(rook_causing_check(game, kingToCheckSafteyFor, teamPieces))
                return bishop_causing_check(game, kingToCheckSafteyFor, teamPieces);
            break;
        case(BISHOP):
            return bishop_causing_check(game, kingToCheckSafteyFor, teamPieces);
            break;
        case(KNIGHT):
            // For bishop, we just have to check if any teamPieces can attack the knight
            for(GameSqaure* teamPiece: teamPieces){
                if(verifyMove(game, *teamPiece, game.pieceCausingKingCheck))
                    return false; // Some team piece can attack the knight, no gameover
            }
            break;
        default:
            break; // Idk what to do if the king is causing the check? if its a pawn we dont have to check, because it can only attack if its 1 spot away and diagonally
    }

    return true; // This determines if the game is over or not
}


// True --> Valid
// False --> Invalid
bool verifyMove(ChessGame &game, GameSqaure &from, GameSqaure &to){
    if(validateMoveset(game, from, to))
        return unobstructed_path_check(game, from, to);
    return false;
}

GameSqaure* moveConverter(ChessGame &game, std::wstring& move){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top

    //!TESTING
    wchar_t a = move[0];
    wchar_t b = move[1];

    GameSqaure* temp = &game.GameBoard[8 - (b - 48)][a - 97];

    return &game.GameBoard[8 - (move[1] - 48)][move[0] - 97];
}

static char toLowercase(char ch){
    if(std::isupper(ch))
        ch = std::tolower(ch);
    return ch;
}



// 0 --> Good
// 1 --> Game changing option (requires restart) // not implemented yet
// 2 --> Invalid input
int getMove(std::wstring& dst){

    bool optionMenu = false;
    // get input
    std::wcin >> dst;
    
    if(!std::isalpha(dst[0])) 
        return 2; //! make sure [0] is alphabetical character
    
    // force to lower
    dst[0] = std::tolower(dst[0]); //! force [0] to lower case
    switch(dst[0]){
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
            optionMenu = !optionMenu;
            break;
        default: //! make sure one of the character above
            return 2;

    }

    if(optionMenu){
        handleOption();
        // depending on the return if will tell main if it requires the,
        // game to be stop or restarted
        
        // for ex, if change color, we need to do that then just continue
        // but like restart game we need to return 1 to tell main to handle
        return 1; //! REDO TURN
    }

    // make sure length is 2
    if(dst.length() != 2)
        return 2; //! move must be length of 2

    // make sure 2 char is a number
    if(!std::isdigit(dst[1]) || dst[1] == L'0' || dst[1] == L'9') // using std::isdigit
        return 2; //! make sure [1] must be a digit and not '0' OR '9' 

    // Now we know it must be [0] == a-h, [1] == 1 - 8

    dst.append(L"\0"); //! add null terminator in case we want to print
    return 0;
    
}

void uninit_moveset(){
    if(Pawn1.moves == nullptr){
        // only if a local game ever init_moveset
    }
}


void init_moveset(){
    //^ Dependencies like pon moving 2 up only on first turn will be in last index
    // mov 1 up, mov 1 up right 1, mov 1 up left 1, mov 2 up
    Pawn1.moves = new Move[PAWN_POSSIBLE_MOVES]; 
    // mov 1 dw, mov 1 dw right 1, mov 1 dw left 1, mov 2 dw
    Pawn2.moves = new Move[PAWN_POSSIBLE_MOVES]; 
    // 2 up 1 right ...
    Knight.moves = new Move[KNIGHT_POSSIBLE_MOVES]; 
    // right up, left down, left up, right down (each 7)
    Bishop.moves = new Move[BISHOP_POSSIBLE_MOVES]; // double check this
    // up (7), down (7), left (7), right (7)
    Rook.moves = new Move[BISHOP_POSSIBLE_MOVES];
    // 1 up, 1 up 1 right, 1 up 1 left, 1 left, 1 right, 1 dw, 1 dw 1 left, 1 dw 1 right
    King.moves = new Move[KING_POSSIBLE_MOVES];
    // Bishop + Rook + King
    Queen.moves = new Move[QUEEN_POSSIBLE_MOVES];

    //! Most important thing to remember
    // origin (0,0) is in the top left not bottom left
    // so going up 1 on a chess board will actually be -1
    // x axis is the same, add 1 will go to the right

                  // x  y
    Pawn1.moves[0] = {0, -1};
    Pawn1.moves[1] = {1, -1};
    Pawn1.moves[2] = {-1, -1};
    Pawn1.moves[3] = {0, -2};

                  // x  y
    Pawn2.moves[0] = {0, 1};
    Pawn2.moves[1] = {-1, 1};
    Pawn2.moves[2] = {1, 1};
    Pawn2.moves[3] = {0, 2};

                    // x  y
    Knight.moves[0] = {1, 2};
    Knight.moves[1] = {2, 1};
    Knight.moves[2] = {2, -1};
    Knight.moves[3] = {1, -2};
    Knight.moves[4] = {-1, -2};
    Knight.moves[5] = {-2, -1};
    Knight.moves[6] = {-2, 1};
    Knight.moves[7] = {-1, 2};
    
    // x  y
    // keep in case i fucked something up

    /*
    for(short i = 1; i < 8; i++){
        std::wcout << "Bishop.moves" << "[" << i - 1 << "]" << "=" << "{" << i << "," << i << "};" << std::endl;
    }

    for(short j = -1; j > -8; j--){
        std::wcout << "Bishop.moves" << "[" << abs(j) - 1 + 7 << "]" << "=" << "{" << j << "," << j << "};" << std::endl;
    }

    for(short k = -1; k > -8; k--){
        std::wcout << "Bishop.moves" << "[" << abs(k) - 1 + 14 << "]" << "=" << "{" << k << "," << abs(k) << "};" << std::endl;
    }

    for(short l = -1; l > -8; l--){
        std::wcout << "Bishop.moves" << "[" << abs(l) - 1 + 21 << "]" << "=" << "{" << abs(l) << "," << l << "};" << std::endl;
    }
    */

    Bishop.moves[0]={1,1}; // bottom left to top right
    Bishop.moves[1]={2,2};
    Bishop.moves[2]={3,3};
    Bishop.moves[3]={4,4};
    Bishop.moves[4]={5,5};
    Bishop.moves[5]={6,6};
    Bishop.moves[6]={7,7};
    Bishop.moves[7]={-1,-1}; // top right to bottom left
    Bishop.moves[8]={-2,-2};
    Bishop.moves[9]={-3,-3};
    Bishop.moves[10]={-4,-4};
    Bishop.moves[11]={-5,-5};
    Bishop.moves[12]={-6,-6};
    Bishop.moves[13]={-7,-7};
    Bishop.moves[14]={-1,1}; // bottom right to top left
    Bishop.moves[15]={-2,2};
    Bishop.moves[16]={-3,3};
    Bishop.moves[17]={-4,4};
    Bishop.moves[18]={-5,5};
    Bishop.moves[19]={-6,6};
    Bishop.moves[20]={-7,7};
    Bishop.moves[21]={1,-1}; // top left to bottom right
    Bishop.moves[22]={2,-2};
    Bishop.moves[23]={3,-3};
    Bishop.moves[24]={4,-4};
    Bishop.moves[25]={5,-5};
    Bishop.moves[26]={6,-6};
    Bishop.moves[27]={7,-7};

    // keep in case of fuck up
    /*
    for(short i = 1; i < 8; i++){
        std::wcout << "Rook.moves" << "[" << i - 1 << "]" << "=" << "{" << i << "," << 0 << "};" << std::endl;
    }

    for(short j = 1; j < 8; j++){
        std::wcout << "Rook.moves" << "[" << j - 1 + 7 << "]" << "=" << "{" << 0 << "," << j << "};" << std::endl;
    }

    for(short k = -1; k > -8; k--){
        std::wcout << "Rook.moves" << "[" << abs(k) - 1 + 14 << "]" << "=" << "{" << k << "," << 0 << "};" << std::endl;
    }

    for(short l = -1; l > -8; l--){
        std::wcout << "Rook.moves" << "[" << abs(l) - 1 + 21 << "]" << "=" << "{" << 0 << "," << l << "};" << std::endl;
    }
    */

    Rook.moves[0]={1,0}; // bottom left to bottom right
    Rook.moves[1]={2,0};
    Rook.moves[2]={3,0};
    Rook.moves[3]={4,0};
    Rook.moves[4]={5,0};
    Rook.moves[5]={6,0};
    Rook.moves[6]={7,0};
    Rook.moves[7]={0,1}; // bottom left to top left
    Rook.moves[8]={0,2};
    Rook.moves[9]={0,3};
    Rook.moves[10]={0,4};
    Rook.moves[11]={0,5};
    Rook.moves[12]={0,6};
    Rook.moves[13]={0,7};
    Rook.moves[14]={-1,0}; // top right to top left
    Rook.moves[15]={-2,0};
    Rook.moves[16]={-3,0};
    Rook.moves[17]={-4,0};
    Rook.moves[18]={-5,0};
    Rook.moves[19]={-6,0};
    Rook.moves[20]={-7,0};
    Rook.moves[21]={0,-1}; // top right to bottom right
    Rook.moves[22]={0,-2};
    Rook.moves[23]={0,-3};
    Rook.moves[24]={0,-4};
    Rook.moves[25]={0,-5};
    Rook.moves[26]={0,-6};
    Rook.moves[27]={0,-7};

    King.moves[0] = {0,1};
    King.moves[1] = {1,1};
    King.moves[2] = {1,0};
    King.moves[3] = {1,-1};
    King.moves[4] = {0,-1};
    King.moves[5] = {-1,-1};
    King.moves[6] = {-1,0};
    King.moves[7] = {-1,1};

    // Just copy bishop + rook + king but change names and indexes
    // i think...

    Queen.moves[0] = {1,1};
    Queen.moves[1] = {2,2};
    Queen.moves[2] = {3,3};
    Queen.moves[3] = {4,4};
    Queen.moves[4] = {5,5};
    Queen.moves[5] = {6,6};
    Queen.moves[6] = {7,7};
    Queen.moves[7] = {-1,-1};
    Queen.moves[8] = {-2,-2};
    Queen.moves[9] = {-3,-3};
    Queen.moves[10] = {-4,-4};
    Queen.moves[11] = {-5,-5};
    Queen.moves[12] = {-6,-6};
    Queen.moves[13] = {-7,-7};
    Queen.moves[14] = {-1,1};
    Queen.moves[15] = {-2,2};
    Queen.moves[16] = {-3,3};
    Queen.moves[17] = {-4,4};
    Queen.moves[18] = {-5,5};
    Queen.moves[19] = {-6,6};
    Queen.moves[20] = {-7,7};
    Queen.moves[21] = {1,-1};
    Queen.moves[22] = {2,-2};
    Queen.moves[23] = {3,-3};
    Queen.moves[24] = {4,-4};
    Queen.moves[25] = {5,-5};
    Queen.moves[26] = {6,-6};
    Queen.moves[27] = {7,-7};
    Queen.moves[28] = {1,0};
    Queen.moves[29] = {2,0};
    Queen.moves[30] = {3,0};
    Queen.moves[31] = {4,0};
    Queen.moves[32] = {5,0};
    Queen.moves[33] = {6,0};
    Queen.moves[34] = {7,0};
    Queen.moves[35] = {0,1};
    Queen.moves[36] = {0,2};
    Queen.moves[37] = {0,3};
    Queen.moves[38] = {0,4};
    Queen.moves[39] = {0,5};
    Queen.moves[40] = {0,6};
    Queen.moves[41] = {0,7};
    Queen.moves[42] = {-1,0};
    Queen.moves[43] = {-2,0};
    Queen.moves[44] = {-3,0};
    Queen.moves[45] = {-4,0};
    Queen.moves[46] = {-5,0};
    Queen.moves[47] = {-6,0};
    Queen.moves[48] = {-7,0};
    Queen.moves[49] = {0,-1};
    Queen.moves[50] = {0,-2};
    Queen.moves[51] = {0,-3};
    Queen.moves[52] = {0,-4};
    Queen.moves[53] = {0,-5};
    Queen.moves[54] = {0,-6};
    Queen.moves[55] = {0,-7};
    Queen.moves[56] = {0,1};
    Queen.moves[57] = {1,1};
    Queen.moves[58] = {1,0};
    Queen.moves[59] = {1,-1};
    Queen.moves[60] = {0,-1};
    Queen.moves[61] = {-1,-1};
    Queen.moves[62] = {-1,0};
    Queen.moves[63] = {-1,1};
}