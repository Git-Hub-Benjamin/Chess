#include "../chess.h"
#include "../server/socket/socketutil.h"
#include <unistd.h>

void online_game(){
    int playerSocketFD = createTCPIPv4Socket();     // server info
    struct sockaddr_in* serverAddr = createIPv4Address("127.0.0.1", 2000);

    int res = connect(playerSocketFD, (const sockaddr*)serverAddr, sizeof(*serverAddr));
    if(res != 0){
        std::wcout << "Connection to server failure." << std::endl;
        exit(EXIT_FAILURE);
    }

    send(playerSocketFD, "Hello from chess client", sizeof("Hello from chess client"), 0);

    while(true){
        sleep(1);
        std::wcout << "Waiting..." << std::endl;
    }
}

void local_game(){
    //! Required
    if(Pawn1.moves == nullptr)
        init_moveset();
    
    ChessGame Game;

    // White alwalys go first
    bool* game_gameover = &Game.gameover;

    while(!*game_gameover)
    {
        
        print_board(Game);
        if(Game.currentTurn == PONE)
            std::wcout << "Player one's turn..." << std::endl;
        else
            std::wcout << "Player twos's turn..." << std::endl;

        bool validMove = false;
        while(!validMove)
        {
            //? defined in the order of usage
            std::wstring move;
            GameSqaure* movePiece;
            std::wstring moveTo;
            GameSqaure* moveToSquare;
            bool attemptToTake;
            int res = getMove(move, true);
            if(res != 0){
                // handle
            }
            // move --> gamesquare pointer of the gameboard
            movePiece = moveConverter(Game, move);

            // if there is no piece at the point trying to move from
            if(movePiece->piece == OPEN || movePiece->ownership == NONE){
                std::wcout << L"No piece present." << std::endl;
                continue; //! cannot move a piece of OPEN, cannot move an open square
            }

            // make sure piece belongs to current turn
            if(movePiece->ownership != Game.currentTurn){
                std::wcout << L"Piece does not belong to you." << std::endl;
                continue; //! cannot move a square of the other players piece
            }
            // We know the move from square is valid now lets get the moveTo

            res = getMove(moveTo, false);
            if(res != 0){
                // handle
            }
            
            // moveTo --> gamesquare pointer to point on board
            moveToSquare = moveConverter(Game, moveTo);

            // make sure that the movetosquare owner is not equal to the current turn
            //? the only exception is for castling
            if(moveToSquare->ownership == Game.currentTurn){
                std::wcout << L"Cannot take own piece." << std::endl;
                // hanlde castling later, its complex...
                continue; //! cannot take own piece
            }
            
            // if this is true then we know its trying to take a piece
            if(moveToSquare->ownership != NONE)
                attemptToTake = true;

            // now verify that this move will be in bounds...
            res = verifyMove(Game.currentTurn, *movePiece, *moveToSquare);
            if(res != 0){
                //! Invalid move
                std::wcout << L"Invalid move." << std::endl;
                continue;
            }

            validMove = true;
            res = makeMove(Game, *movePiece, *moveToSquare);
            if(res == 2){
                continue; //! I dont know when makemove would return 2
            }else{
                if(res == 1)
                    std::wcout << "Piece taken." << std::endl;
            }
        }
        // Switch turns
        Game.currentTurn = Game.currentTurn == PONE ? PTWO : PONE;
    }
}