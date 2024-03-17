#include "chess.h"
#include <string>

extern void init_moveset();
extern struct Piece_moveset Pon1;
extern struct Piece_moveset Pon2;
extern struct Piece_moveset Knight;
extern struct Piece_moveset Bishop;
extern struct Piece_moveset Rook;
extern struct Piece_moveset Queen;
extern struct Piece_moveset King;

int PIECE_MOVE_COUNTS[] = {PON_POSSIBLE_MOVES, KNIGHT_POSSIBLE_MOVES, BISHOP_POSSIBLE_MOVES, ROOK_POSSIBLE_MOVES, KING_POSSIBLE_MOVES, QUEEN_POSSIBLE_MOVES};

struct Piece_moveset* PIECE_MOVES[] = {&Pon2, &Pon1, &Knight, &Bishop, &Rook, &King, &Queen};

// wchar_t piece_art_p1[7] = {' ', L'♟', L'♞', L'♝', L'♜', L'♚', L'♛'};
// wchar_t piece_art_p2[7] = {' ', L'♙', L'♘', L'♗', L'♖', L'♔', L'♕'};

wchar_t piece_art_p1[7] = {' ', L'P', L'K', L'B', L'R', L'K', L'Q'};
wchar_t piece_art_p2[7] = {' ', L'p', L'k', L'b', L'r', L'k', L'q'};


void print_board(ChessGame &game){
    std::wcout << L"    a   b   c   d   e   f   g   h\n" 
    << L"  +---+---+---+---+---+---+---+---+\n";
    for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
        std::wcout << CHESS_BOARD_HEIGHT - i << L" ";
        for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

            // Get piece
            wchar_t piece;
            if(game.GameBoard[i][j].ownership == NONE)
                piece = ' ';
            else if(game.GameBoard[i][j].ownership == PONE)
                piece = piece_art_p1[game.GameBoard[i][j].piece];
            else
                piece = piece_art_p2[game.GameBoard[i][j].piece];

            // print section --> "| ♟ " ex.
            std::wcout << L"| " << piece << L" ";
        }
        std::wcout << L"| " << CHESS_BOARD_HEIGHT - i << std::endl;
        std::wcout << L"  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << L"    a   b   c   d   e   f   g   h\n";
}

int verifyMove(ChessGame &game, GameSqaure &from, GameSqaure &to){

    game.GameBoard[to.pos.y][to.pos.x].piece == from.piece;
    game.GameBoard[to.pos.y][to.pos.y].ownership == game.currentTurn;
    game.GameBoard[from.pos.y][from.pos.x].ownership == NONE;
    game.GameBoard[from.pos.y][from.pos.x].piece == OPEN;

    // we know from has a piece present
    //* SETUP
                                                // -1  since we dont use OPEN (0)
    short possibleMoveCounter = PIECE_MOVE_COUNTS[from.piece - 1];
    enum GamePiece piece = from.piece;
    struct Piece_moveset* PIECE_MOVESET = PIECE_MOVES[from.piece];
    bool VALID_MOVE = false;
    std::cout << "this is a test" << std::endl;
    fflush(stdout);
    
    // if its a pon, and its the player two turn then set this bc ptwo has
    // a different moveset for the pon
    if(game.currentTurn == PTWO) 
        PIECE_MOVESET = PIECE_MOVES[0];

    struct Point pointToGetTo = to.pos; // make a copy of to
    
    for(int i = 0; i < possibleMoveCounter; i++){
        // since we are testing this every time we need to make a copy everytime
        struct Point pointToMoveFrom = from.pos; // make a copy of from

        pointToMoveFrom.x += PIECE_MOVESET->moves[i].x;
        pointToMoveFrom.y += PIECE_MOVESET->moves[i].y;
        if(pointToMoveFrom.x == pointToGetTo.x && pointToMoveFrom.y == pointToGetTo.y){
            // This move will work
            VALID_MOVE = true;
            break;
        }
    }

    //std::cout << "Valid? --> " << VALID_MOVE << std::endl;

    game.GameBoard[to.pos.y][to.pos.x].piece == from.piece;
    game.GameBoard[to.pos.y][to.pos.y].ownership == game.currentTurn;
    game.GameBoard[from.pos.y][from.pos.x].ownership == NONE;
    game.GameBoard[from.pos.y][from.pos.x].piece == OPEN;

    if(!VALID_MOVE){
        std::cout << "Not valid move." << std::endl;
        return 1;
    }else{
        // Lets move the piece now
        game.GameBoard[to.pos.y][to.pos.x].piece == from.piece;
        game.GameBoard[to.pos.y][to.pos.y].ownership == game.currentTurn;
        game.GameBoard[from.pos.y][from.pos.x].ownership == NONE;
        game.GameBoard[from.pos.y][from.pos.x].piece == OPEN;
    }
    

    return 0;
}

GameSqaure* moveConverter(ChessGame &game, std::string& move){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top
    return &game.GameBoard[8 - (move[1] - 48)][move[0] - 97];
}

char toLowercase(char ch){
    if(std::isupper(ch))
        std::tolower(ch);
    return ch;
}

void handleOption(){
    //implement later
}

int getMove(ChessGame game, std::string& dst){
    while(true){
        bool optionMenu = false;
        // get input
        std::cin >> dst;
        
        if(!std::isalpha(dst[0])) 
            continue; //! make sure [0] is alphabetical character
        
        // force to lower
        dst[0] = std::tolower(dst[0]); //! force [0] to lower case
        switch(dst[0]){
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h': //* ALL VALID
                break;
            case 'q': //* VALID too but for option
                optionMenu = !optionMenu;
                break;
            default: //! make sure one of the character above
                continue;

        }

        if(optionMenu){
            handleOption();
            // depending on the return if will tell main if it requires the,
            // game to be stop or restarted
            
            // for ex, if change color, we need to do that then just continue
            // but like restart game we need to return 1 to tell main to handle
            continue; //! REDO TURN
        }

        // make sure length is 2
        if(dst.length() != 2)
            continue; //! move must be length of 2

        // make sure 2 char is a number
        if(!std::isdigit(dst[1]) || dst[1] == '0' || dst[1] == '9') // using std::isdigit
            continue; //! make sure [1] must be a digit and not '0' OR '9' 

        // Now we know it must be [0] == a-h, [1] == 1 - 8
        dst += '\0'; //! add null terminator in case we want to print
        break;
    }
    return 0;
}


static bool running = true;

int main()
{
    //! Required
    init_moveset();

    ChessGame Game;
    print_board(Game);
    std::string move;
    GameSqaure* movePiece;
    std::string moveTo;
    GameSqaure* moveToSquare;
    bool attemptToTake;
    int res = getMove(Game, move);
    movePiece = moveConverter(Game, move);
    res = getMove(Game, moveTo);
    moveToSquare = moveConverter(Game, moveTo);
    res = verifyMove(Game, *movePiece, *moveToSquare);
    print_board(Game);

    return 0;

    while(running)
    {
        ChessGame Game;
        // White alwalys go first
        bool* game_gameover = &Game.gameover;
        while(!*game_gameover)
        {
            bool validMove = false;
            while(!validMove)
            {
                //? defined in the order of usage
                std::string move;
                GameSqaure* movePiece;
                std::string moveTo;
                GameSqaure* moveToSquare;
                bool attemptToTake;
                int res = getMove(Game, move);
                if(res != 0){
                    // handle
                }
                // move --> gamesquare pointer of the gameboard
                movePiece = moveConverter(Game, move);

                // if there is no piece at the point trying to move from
                if(movePiece->piece == OPEN)
                    continue; //! cannot move a piece of OPEN

                //? Technically we only need to check this
                // make sure piece belongs to current turn
                if(movePiece->ownership == NONE 
                    || movePiece->ownership != Game.currentTurn)
                    continue; //! cannot move an open square OR move a square of the other players piece

                // We know the move from square is valid now lets get the moveTo

                res = getMove(Game, moveTo);
                if(res != 0){
                    // handle
                }
                
                // moveTo --> gamesquare pointer to point on board
                moveToSquare = moveConverter(Game, moveTo);

                // make sure that the movetosquare owner is not equal to the current turn
                //? the only exception is for castling
                if(moveToSquare->ownership == Game.currentTurn)
                    // hanlde castling later, its complex...
                    continue; //! cannot take own piece
                
                // if this is true then we know its trying to take a piece
                if(moveToSquare->ownership != NONE)
                    attemptToTake = true;

                // now verify that this move will be in bounds...
                
            }
            // 
        }
    }
}


/*

  a   b   c   d   e   f   g   h
+---+---+---+---+---+---+---+---+
| ♜ | ♞ | ♝ | ♛ | ♚ | ♝ | ♞ | ♜ | 8
+---+---+---+---+---+---+---+---+
| ♟ | ♟ | ♟ | ♟ | ♟ | ♟ | ♟ | ♟ | 7
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 6
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 5
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 4
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   | 3
+---+---+---+---+---+---+---+---+
| ♙ | ♙ | ♙ | ♙ | ♙ | ♙ | ♙ | ♙ | 2
+---+---+---+---+---+---+---+---+
| ♖ | ♘ | ♗ | ♕ | ♔ | ♗ | ♘ | ♖ | 1
+---+---+---+---+---+---+---+---+
  a   b   c   d   e   f   g   h

*/