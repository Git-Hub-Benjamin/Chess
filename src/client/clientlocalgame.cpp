#include "../chess.hpp"
#include "./option.hpp"
#include "client-terminal-frontend/displaymanager.hpp"
#include <sstream>

// GLOBAL
extern Options global_player_option;

// QUEUE
std::wstring nextTurnPrint; 

void print_last_turn_msg(){
    set_terminal_color(RED);
    std::wcout << nextTurnPrint << std::endl;
    set_terminal_color(DEFAULT);
    nextTurnPrint.clear();
}


void local_game(bool dev_mode){
    
    ChessGame Game(dev_mode);
    Game.GameOptions = global_player_option;

    // White alwalys go first
    bool& game_gameover = Game.gameover;

    while(!game_gameover)
    {
        // if(clearQueue)
        //     nextTurnPrint.clear();

        bool CURRENT_TURN_IN_CHECK = false;
        bool QUIT = false;

        if(!kingSafe(Game)){
            if(!checkMate(Game)){ // Check checkmate for the current players turn
                CURRENT_TURN_IN_CHECK = true;
            }else{
                print_board(Game);
                std::wcout << "Player " << (Game.currentTurn == PONE ? "one" : "two") << ", you got check mated!" << std::endl;
                std::wcout << "Player " << (Game.currentTurn == PONE ? "two" : "one") << ", you win!!" << std::endl;
                Game.gameover = true;

                std::wcout << L"Do you want to reset the game? (y/n): ";

                wchar_t in = std::wcin.get(); // Read a wide character from std::wcin
                std::wcin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n'); // Clear the input buffer

                if (in == L'y' || in == L'Y') {
                    Game.reset();
                    Game.currentTurn = PONE;
                } else {
                    break;
                }
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

            if(Game.GameOptions.moveHighlighting){
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
            }

            // We know the move from square is valid now lets get the moveTo
            // Before printing "To: " if highlighting is on then we need to reprint messages
            
            while(true){
                std::wcout << (Game.currentTurn == PONE ? "P1 - " : "P2 - ") <<
                "To: ";
                res = getMove(moveTo);
                if(res != 0){ 
                    // handle
                    if(res == 1){ // 1 --> Option change, 
                        QUIT = true;
                    }else{
                        continue; // 2 --> Invalid input
                    }
                }
                break;
            }

            if(QUIT){
                Game.gameover = true;
                break;
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
            if(!res){
                //! Invalid move
                nextTurnPrint = std::wstring(L"Piece cannot reach there.");
                continue;
            }

            // After making the move we need to see if making this move made a path for the oppenent to put your king in check, so save the GameSquare
            // that we are updating, update it, then check the kings safety, if it makes the players king unsafe, then revert the move

            if(!king_safe_after_move(Game, *movePiece, *moveToSquare, &nextTurnPrint))
                continue;

            break;
            // Switch turns
        }
        Game.currentTurn = Game.currentTurn == PONE ? PTWO : PONE;
    }
    std::wcout << "Game Over..." << std::endl;
}