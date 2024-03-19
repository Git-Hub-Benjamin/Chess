#include "chess.h"

// Conversion from string to wstring
std::wstring convertString(std::string &passed){
    return std::wstring(passed.begin(), passed.end());
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

// wchar_t piece_art_p1[7] = {' ', L'♟', L'♞', L'♝', L'♜', L'♚', L'♛'};
// wchar_t piece_art_p2[7] = {' ', L'♙', L'♘', L'♗', L'♖', L'♔', L'♕'};

wchar_t piece_art_p1[7] = {' ', L'P', L'K', L'B', L'R', L'K', L'Q'};
wchar_t piece_art_p2[7] = {' ', L'p', L'k', L'b', L'r', L'k', L'q'};

// Class functions

ChessGame::ChessGame(){
    gameover = false;
    init();
}

void ChessGame::reset(){
    init();
}

// 0 Good, 1 Bad
void ChessGame::init(){
    currentTurn = PONE;
    memset(&GameBoard, 0, sizeof(GameBoard));

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            GameBoard[row][col].pos = {col, row};
            if(row < Y2) GameBoard[row][col].ownership = PTWO;
            if(row > Y5) GameBoard[row][col].ownership = PONE;
            if(row == Y0 || row == Y7){

                enum GamePiece temp;

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
        }
    }
}

// Logic functions

void print_board(ChessGame &game){
    
    std::wcout << L"\t\t    a   b   c   d   e   f   g   h\n" 
    << L"\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int i = 0; i < CHESS_BOARD_HEIGHT; i++){
        std::wcout << "\t\t" << CHESS_BOARD_HEIGHT - i << L" ";
        for(int j = 0; j < CHESS_BOARD_WIDTH; j++){

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
        std::wcout << L"\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << L"\t\t    a   b   c   d   e   f   g   h\n";
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

    // Lets move the piece now
    game.GameBoard[to.pos.y][to.pos.x].piece = from.piece;
    game.GameBoard[to.pos.y][to.pos.x].ownership = from.ownership;
    game.GameBoard[from.pos.y][from.pos.x].piece = OPEN;
    game.GameBoard[from.pos.y][from.pos.x].ownership = NONE;

    
    return res;

}

// 0 --> Valid
// 1 --> Invalid
int verifyMove(enum Owner currentTurn, GameSqaure &from, GameSqaure &to){

    // we know from has a piece present
    //* SETUP
                                                // -1  since we dont use OPEN (0)
    short possibleMoveCounter = PIECE_MOVE_COUNTS[from.piece - 1];
    enum GamePiece piece = from.piece;
    struct Piece_moveset* PIECE_MOVESET = PIECE_MOVES[from.piece];
    bool VALID_MOVE = false;
    
    // if its a pon, and its the player two turn then set this bc ptwo has
    // a different moveset for the pon
    if(currentTurn == PTWO) 
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

    if(!VALID_MOVE){
        return 1;
    } 

    return 0;
}

GameSqaure* moveConverter(ChessGame &game, std::wstring& move){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top
    return &game.GameBoard[8 - (move[1] - 48)][move[0] - 97];
}

static char toLowercase(char ch){
    if(std::isupper(ch))
        ch = std::tolower(ch);
    return ch;
}

// 0 --> Good
// 1 --> Game changing option (requires restart) // not implemented yet
int getMove(std::wstring& dst, bool firstMove){
    
    while(true){
        if(firstMove)
            std::wcout << "Piece to move: ";
        else
            std::wcout << "To move to: ";

        bool optionMenu = false;
        // get input
        std::wcin >> dst;
        
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

        dst.append(L"\0"); //! add null terminator in case we want to print
        break;
    }
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
        std::cout << "Bishop.moves" << "[" << i - 1 << "]" << "=" << "{" << i << "," << i << "};" << std::endl;
    }

    for(short j = -1; j > -8; j--){
        std::cout << "Bishop.moves" << "[" << abs(j) - 1 + 7 << "]" << "=" << "{" << j << "," << j << "};" << std::endl;
    }

    for(short k = -1; k > -8; k--){
        std::cout << "Bishop.moves" << "[" << abs(k) - 1 + 14 << "]" << "=" << "{" << k << "," << abs(k) << "};" << std::endl;
    }

    for(short l = -1; l > -8; l--){
        std::cout << "Bishop.moves" << "[" << abs(l) - 1 + 21 << "]" << "=" << "{" << abs(l) << "," << l << "};" << std::endl;
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
        std::cout << "Rook.moves" << "[" << i - 1 << "]" << "=" << "{" << i << "," << 0 << "};" << std::endl;
    }

    for(short j = 1; j < 8; j++){
        std::cout << "Rook.moves" << "[" << j - 1 + 7 << "]" << "=" << "{" << 0 << "," << j << "};" << std::endl;
    }

    for(short k = -1; k > -8; k--){
        std::cout << "Rook.moves" << "[" << abs(k) - 1 + 14 << "]" << "=" << "{" << k << "," << 0 << "};" << std::endl;
    }

    for(short l = -1; l > -8; l--){
        std::cout << "Rook.moves" << "[" << abs(l) - 1 + 21 << "]" << "=" << "{" << 0 << "," << l << "};" << std::endl;
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