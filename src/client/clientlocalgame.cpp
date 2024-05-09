#include "../chess.h"

void local_game(bool dev_mode){
    //! Required
    if(Pawn1.moves == nullptr)
        init_moveset();
    
    ChessGame Game(dev_mode);

    // White alwalys go first
    bool& game_gameover = Game.gameover;

    while(!game_gameover)
    {
        
        print_board(Game);

        if(Game.currentTurn == PONE)
            std::wcout << "Player one's turn..." << std::endl;
        else
            std::wcout << "Player twos's turn..." << std::endl;

        if(!kingSafe(Game)){
            if(!checkMate(Game)){ // Check checkmate for the current players turn
                std::wcout << "Player " << (Game.currentTurn == PONE ? "one" : "two") << ", you are in check!" << std::endl; 
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
            std::wstring move;
            GameSqaure* movePiece;
            std::wstring moveTo;
            GameSqaure* moveToSquare;

            int res = getMove(move, true);
            if(res != 0){
                // handle
            }
            // move --> gamesquare pointer of the gameboard
            movePiece = moveConverter(Game, move);

            // Makes sure that the piece you selected to move is valid. (Not Open and Not oppenents piece)
            std::wstring msg;
            validateMovePiece(Game, *movePiece, msg);
            if(msg.length() != 0){
                std::wcout << msg << std::endl;
                continue;
            }
            
            // We know the move from square is valid now lets get the moveTo

            res = getMove(moveTo, false);
            if(res != 0){
                // handle
            }
            
            // moveTo --> gamesquare pointer to point on board
            const wchar_t* wTest = moveTo.c_str();
            moveToSquare = moveConverter(Game, moveTo);

            // Makes sure that the piece you are moving to is not owned by your self, (ONLY EXCPETION IS CASTLING (IMPLEMENT LATER))
            //? IMPLEMENT CASTLING
            validateMoveToPiece(Game, *moveToSquare, msg);
            if(msg.length() != 0){
                std::wcout << msg << std::endl;
                continue;
            }

            // Now we need to make sure that the piece can actually make this move
            res = verifyMove(Game, *movePiece, *moveToSquare);
            if(res != 1){
                //! Invalid move
                std::wcout << "Invalid move." << std::endl;
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
                    if(res == 0)
                        std::wcout << "Piece moved." << std::endl;
                    else
                        std::wcout << "Piece taken." << std::endl;                    
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