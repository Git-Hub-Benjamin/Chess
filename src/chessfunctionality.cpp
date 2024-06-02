#include "chess.hpp"
#include <vector>
#include <unordered_map>
#include <array>

std::array<int, 40> a;

// Conversion from string to wstring
std::wstring convertString(const std::string& passed) {
    return std::wstring(passed.begin(), passed.end());
}

// conversion from wstring to string
std::string convertWString(std::wstring& passed){
    return std::string(passed.begin(), passed.end());
}

// Logic functions



// True  -> nothing can stop this attack >:)
// False -> means that not checkmate since a piece can block OR take the rook 
static bool rook_causing_check(ChessGame &game, GameSqaure& checkedKing, std::vector<GameSqaure*>& teamPieces){
    GameSqaure temp = *game.pieceCausingKingCheck;
                                                                        // if on same x axis, then moving up or down, 
    int xdir = (game.pieceCausingKingCheck->pos.x == checkedKing.pos.x) ? (game.pieceCausingKingCheck->pos.x - checkedKing.pos.x < 0 ? -1 : 1) : 0;
                // if x == 0 then it must be on the same y axis value, so going left or right, 
    int ydir = xdir == 0 ? game.pieceCausingKingCheck->pos.y - checkedKing.pos.y < 0 ? 1 : -1 : 0;
    int amount_to_check = xdir == 0 ? std::abs(checkedKing.pos.y - game.pieceCausingKingCheck->pos.y) : std::abs(checkedKing.pos.x - game.pieceCausingKingCheck->pos.x);

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


bool onBoard(Point& p){
    return (p.x <= 7 && p.x >= 0 && p.y <= 7 && p.y >= 0);
}


static bool check_king_safety_for_board_square(ChessGame &game, GameSqaure &to){
    
    // We need to pretend that the king is not present on the board during these checks

    game.KingPositions[game.currentTurn - 1]->ownership = NONE;
    game.KingPositions[game.currentTurn - 1]->piece = OPEN;
    bool res = true;

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            GameSqaure& currentSquare = game.GameBoard[row][col];
            if(currentSquare.ownership == NONE || currentSquare.piece == OPEN || currentSquare.ownership == game.currentTurn)
                continue;
            
            if(verifyMove(game, currentSquare, to)){ 
                // if this returns true then it means that the currentSquare can actually attack the king, 
                //! BUT this doesnt apply for pawns, we need to pawns we need to do extra check for them 
                res = false; // KING IS NOT SAFE
                goto out;
            }
        }
    }

out:
    // Restore king position
    game.KingPositions[game.currentTurn - 1]->ownership = game.currentTurn;
    game.KingPositions[game.currentTurn - 1]->piece = KING;
    return res;
}



std::vector<GameSqaure*>* get_move_to_squares(ChessGame &game, GameSqaure& from) {
    std::vector<GameSqaure*>* squares = new std::vector<GameSqaure*>;

    // SETUP --> very similar to verifyMove function
    // -1 since we don't use OPEN (0)
    short possibleMoveCounter = PIECE_MOVE_COUNTS[from.piece - 1];
    enum GamePiece piece = from.piece;
    struct Piece_moveset* PIECE_MOVESET = PIECE_MOVES[from.piece];

    // if it's a pawn, and it's the player two turn then set this bc ptwo has
    // a different moveset for the pawn
    if (game.currentTurn == PTWO && piece == PAWN) 
        PIECE_MOVESET = PIECE_MOVES[0];

    for (int i = 0; i < possibleMoveCounter; i++) {
        // since we are testing this every time we need to make a copy everytime
        struct Point pointToMoveFrom = from.pos; // make a copy of from

        pointToMoveFrom.x += PIECE_MOVESET->moves[i].x;
        pointToMoveFrom.y += PIECE_MOVESET->moves[i].y;

        // If on board, nothing in the way, and not attacking own teammate, then add it to the vector
        if (onBoard(pointToMoveFrom))
            if (unobstructed_path_check(game, from, game.GameBoard[pointToMoveFrom.y][pointToMoveFrom.x]))
                if (game.GameBoard[pointToMoveFrom.y][pointToMoveFrom.x].ownership != game.currentTurn){
                    if (from.piece == KING && !check_king_safety_for_board_square(game, game.GameBoard[pointToMoveFrom.y][pointToMoveFrom.x]))
                        continue;
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

void print_board_with_moves(ChessGame &game, GameSqaure& from, std::vector<GameSqaure*>& vecOfSquare){
    // first get the gameSquares that the piece can move to into a vector

    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - i << " ";
        for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

            std::wcout << "| ";
            wchar_t piece;
            if(game.GameBoard[i][j].ownership == NONE)
                piece = ' ';
            else if(game.GameBoard[i][j].ownership == PONE){
                piece = piece_art_p1[game.GameBoard[i][j].piece];
                set_terminal_color(game.GameOptions.p1_color);
            }else{
                piece = piece_art_p2[game.GameBoard[i][j].piece];
                set_terminal_color(game.GameOptions.p2_color);
            }

            // checking if the current square can be acctacked by piece
            if(in_vec_squares(&vecOfSquare, game.GameBoard[i][j])){
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
        std::wcout << "| " << CHESS_BOARD_HEIGHT - i << std::endl;
        std::wcout << "\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t    a   b   c   d   e   f   g   h\n";
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
                //! BUT this doesnt apply for pawns, we need to pawns we need to do extra check for them 
                
                game.pieceCausingKingCheck = &currentSquare; // Needed for checkMate
                
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

    // Since we are checking these pieces assuming the king is moving there we need to also assume the king has moved
    kingToCheckSafteyFor.ownership = NONE;
    kingToCheckSafteyFor.piece = OPEN;

    bool res = true;

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

        if(!ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE){
            res = false; // False means that this space is safe because the king can get to it without any enemies reaching the square
            goto restore_king;
        }
    }
restore_king:

    kingToCheckSafteyFor.ownership = game.currentTurn;
    kingToCheckSafteyFor.piece = KING;

    if(!res)
        return false;
    

    // Now we need to see if any of the team pieces can block the enemy piece that is causing the team king to be in check
    switch(game.pieceCausingKingCheck->piece){
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
                if(verifyMove(game, *teamPiece, *game.pieceCausingKingCheck))
                    return false; // Some team piece can attack the knight, no gameover
            }
            break;
        default:
            break; // Idk what to do if the king is causing the check? if its a pawn we dont have to check, because it can only attack if its 1 spot away and diagonally
    }

    return true; // This determines if the game is over or not
}

int char_single_digit_to_int(const char c){
    return c - 48;
}


std::string toLowercase(const std::string& str) {
    std::string result;
    for (char c : str) {
        result += std::tolower(c);
    }
    return result;
}
