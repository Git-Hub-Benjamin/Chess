#include "../chess.hpp"
#include <sstream>

// GLOBAL
extern enum WRITE_COLOR playerOneColor;
extern enum WRITE_COLOR playerTwoColor;
// QUEUE
std::wstring nextTurnPrint; 

void print_last_turn_msg(){
    set_terminal_color(RED);
    std::wcout << nextTurnPrint << std::endl;
    set_terminal_color(DEFAULT);
    nextTurnPrint = L"";
}


void local_game(bool dev_mode){
    //! Required
    if(Pawn1.moves == nullptr)
        init_moveset();
    
    ChessGame Game(dev_mode);
    Game.p1_color = playerOneColor;
    Game.p2_color = playerTwoColor;
    

    // White alwalys go first
    bool& game_gameover = Game.gameover;

    while(!game_gameover)
    {
        // if(clearQueue)
        //     nextTurnPrint.clear();

        bool CURRENT_TURN_IN_CHECK = false;

        if(!kingSafe(Game)){
            if(!checkMate(Game)){ // Check checkmate for the current players turn
                CURRENT_TURN_IN_CHECK = true;
            }else{
                std::wcout << "Player " << (Game.currentTurn == PONE ? "one" : "two") << ", you got check mated!" << std::endl;
                std::wcout << "Player " << (Game.currentTurn == PONE ? "two" : "one") << ", you win!!" << std::endl;
                Game.gameover = true;
                break;
            }
        }                

        while(true)
        {
            //? defined in the order of usage
            int res;
            std::wstring msg;
            std::wstring move;
            GameSqaure* movePiece;
            std::wstring moveTo;
            GameSqaure* moveToSquare;

            if(!CURRENT_TURN_IN_CHECK){
                print_board(Game);
                
                if(Game.currentTurn == PONE)
                    std::wcout << "Player one's turn..." << std::endl;
                else
                    std::wcout << L"Player twos's turn..." << std::endl;

                if(nextTurnPrint.length() != 0)
                    print_last_turn_msg();

                std::wcout << (Game.currentTurn == PONE ? "P1 - " : "P2 - ") << 
                "Move: ";
                res = getMove(move);
                if(res != 0){ 
                    // handle
                    if(res == 1){ // 1 --> Option change, 
                        Game.gameover = true; 
                        break;
                    }else{
                        continue; // 2 --> Invalid input
                    }
                }
                // move --> gamesquare pointer of the gameboard
                movePiece = moveConverter(Game, move);

                // Makes sure that the piece you selected to move is valid. (Not Open and Not oppenents piece)
                validateMovePiece(Game, *movePiece, msg);
                if(msg.length() != 0){
                    nextTurnPrint = std::wstring(msg);
                    continue;
                }
            }else{
                // If the king is in check then we already know that the moveFromSquare / Piece is the king
                movePiece = Game.KingPositions[Game.currentTurn - 1];
            }

            std::vector<GameSqaure*>* squaresPieceCanMoveTo;
            squaresPieceCanMoveTo = get_move_to_squares(Game, *movePiece);
            if(squaresPieceCanMoveTo->size() > 0) 
                print_board_with_moves(Game, *movePiece, *squaresPieceCanMoveTo);
            else{
                nextTurnPrint = std::wstring(L"No valid moves with this piece.");
                continue;
            }

            if(Game.currentTurn == PONE)
                std::wcout << "Player one's turn..." << std::endl;
            else
                std::wcout << L"Player twos's turn..." << std::endl;
            
            if(!CURRENT_TURN_IN_CHECK)
                std::wcout << L"Move: " << move << std::endl;

            if(CURRENT_TURN_IN_CHECK){
                set_terminal_color(RED);
                if(Game.currentTurn == PONE)
                    std::wcout << L"Player one, You are in check!" << std::endl;
                else
                    std::wcout << L"Player two, You are in check!" << std::endl;
                set_terminal_color(DEFAULT);
            }

            // We know the move from square is valid now lets get the moveTo
            // Before printing "To: " if highlighting is on then we need to reprint messages
            
            std::wcout << (Game.currentTurn == PONE ? "P1 - " : "P2 - ") <<
            "To: ";
            res = getMove(moveTo);
            if(res != 0){ 
                // handle
                if(res == 1){ // 1 --> Option change, 

                }else{
                    continue; // 2 --> Invalid input
                }
            }
            
            // moveTo --> gamesquare pointer to point on board
            moveToSquare = moveConverter(Game, moveTo);

            // Makes sure that the piece you are moving to is not owned by your self, (ONLY EXCPETION IS CASTLING (IMPLEMENT LATER))
            //? IMPLEMENT CASTLING
            validateMoveToPiece(Game, *moveToSquare, msg);
            if(msg.length() != 0){
                nextTurnPrint = std::wstring(msg);
                continue;
            }

            // Now we need to make sure that the piece can actually make this move
            res = verifyMove(Game, *movePiece, *moveToSquare);
            if(res != 1){
                //! Invalid move
                nextTurnPrint = std::wstring(msg);
                continue;
            }

            // After making the move we need to see if making this move made a path for the oppenent to put your king in check, so save the GameSquare
            // that we are updating, update it, then check the kings safety, if it makes the players king unsafe, then revert the move

            GameSqaure copyOfSquareBeingMoved = *movePiece;
            GameSqaure copyOfSquareBeingMovedTo = *moveToSquare;

            res = makeMove(Game, *movePiece, *moveToSquare); //! CAUSING SEGMENTATION FAULT, Maybe something to do with copy? it was working fine before...
            if(res == 2)
                continue; //! I dont know when makemove would return 2, but for later if i need it
            else{
                if(kingSafe(Game)){
                    // if(res == 0)
                    //     toPrint.push_front(std::wstring(L"Piece moved.")); 
                    // else
                    //     toPrint.push_front(std::wstring(L"Piece taken."));                 
                }else{
                    makeMove(Game, copyOfSquareBeingMoved, copyOfSquareBeingMovedTo); // Revert move
                    continue;                                                         // Redo move
                }
            }

            break;
            // Switch turns
        }
        Game.currentTurn = Game.currentTurn == PONE ? PTWO : PONE;
    }
    std::wcout << "Game Over..." << std::endl;
}