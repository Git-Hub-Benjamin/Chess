#include "../Chess/chess.hpp"
#include "server.hpp"

void StandardServerOnlineChessGame::formateCurrTurnMove(std::string& in, std::wstring& move, std::wstring& to){
    move += in[CLIENT_MOVE_INDEX_AFTER_COLON];
    move += in[CLIENT_MOVE_INDEX_AFTER_COLON + 1];
    to +=   in[CLIENT_TO_INDEX_AFTER_COLON];
    to +=   in[CLIENT_TO_INDEX_AFTER_COLON + 1];
}

void StandardServerOnlineChessGame::startGame(){

    while (!mainStartGame) {
        std::this_thread::yield();
    }
 
    // Make sure both clients send ("match-start")
    if(!initClientConnection()){
        std::wcout << "client connection was not successful" << std::endl;
        closeSelfLobby();
        return; 
    }

    while(!GameOver){

        set_terminal_color(GREEN);
        std::wcout << "Current Turn Client: " << convertString(currentTurnClient.CLIENT_STRING_ID) << std::endl;
        std::wcout << "Not Current Turn Client: " << convertString(nonCurruentTurnClient.CLIENT_STRING_ID) << std::endl;
        set_terminal_color(DEFAULT);

        //! print_board(Lobby_Game);

        std::wcout << "Test to make print of gamesquare works in server" << std::endl;
        bool CURR_TURN_IN_CHECK = false;

        // Make sure to switch currentTurn at the end of turn as always
        std::string preTurnCheckInMsg;

        if(!kingSafe()){
            if(!checkMate()){ // Check checkmate for the current players turn
                preTurnCheckInMsg = GAMESTATUS_CHECK;
                CURR_TURN_IN_CHECK = true;
            }else{
                preTurnCheckInMsg = GAMESTATUS_CHECKMATE;
                GameOver = true;
                break;
            }
        }else{
            preTurnCheckInMsg = GAMESTATUS_ALL_GOOD;
        }

        //! This is where we would do some checks for GAMESTATUS_DC

        std::wcout << "Server sending pre check in" << std::endl;
        if(!sendPreTurnCheckIn(preTurnCheckInMsg)){
            // Tell clients there was a socket error, Server fault
            set_terminal_color(RED);
            std::wcout << "ERROR -- SOCKET ERROR" << std::endl;
            set_terminal_color(DEFAULT);
            return;
        }
        std::wcout << "Server sent pre turn check in" << std::endl;

        if (GameOver)
            break; // In case checkmate was true

        std::wcout << "Waiting for the non current turn client to send check in" << std::endl;
        
        int res = nonTurnSpecificClientCheckIn();
        if (res <= 0)
            return;

        std::wcout << "Non turn client sent their check in to the server" << std::endl;

        while(true){

            std::string currTurnResponse;
            
            res = recieveCurrTurnMove(currTurnResponse);
            std::wcout << "Response sent from client: " << convertString(currTurnResponse) << std::endl; 
            if(res < 0){
                // Tell clients there was a socket error, Server fault
            }else if(res > 0){
                // Client fault, DC
            }

            std::wstring move;
            std::wstring to;
            formateCurrTurnMove(currTurnResponse, move, to);

            // Got move and to, now check valid
            std::string validatyOfMoveStr;
            
            int moveRes = makeMove(Move(convertMove(move, currentTurn), convertMove(to, currentTurn)));
            if (moveRes == -1)
                validatyOfMoveStr = SERVER_CLIENT_VERIFY_MOVE_INVALID_CHECK;
            else if(moveRes == 0)
                validatyOfMoveStr = SERVER_CLIENT_VERIFY_MOVE_INVALID;
            else
                validatyOfMoveStr = SERVER_CLIENT_VERIFY_MOVE_VALID;
            
            if(moveRes < 1)
                continue;

            std::wcout << "Tell the current turn client the validaty of the move" << std::endl;

            if(send(currentTurnClient.gameFd, (void*)validatyOfMoveStr.c_str(), validatyOfMoveStr.length(), 0) < 0){
                // Tell clients there was a socket error, Server fault
            }

            std::wcout << "Told the current turn client if their move was valid" << std::endl;

            // If its invalid, we have to do it all again
            if(validatyOfMoveStr.compare(SERVER_CLIENT_VERIFY_MOVE_VALID) != 0)
                continue;

            std::wcout << "It was a valid move, now we are going to send that move to the non current turn client" << std::endl;

            // Valid move, the other player is still expecting the move the other player made,
            // Since we already did the checks on it we can just send it

            if (!sendCurrTurnValidatityOfMove(currTurnResponse))
                return;

            std::wcout << "Told the non current client the move" << std::endl;

            break;
        }

        std::wcout << "recieving the end check in from both clients" << std::endl;

        if(!endOfTurnClientsCheckIn()){
            std::wcout << "Client connection died at end turn check." << std::endl;
            return; // So self can join with main thread
        }

        std::wcout << "both clients sent the end check in, continuing to the next game loop" << std::endl;

        // Swap turns, We need to swap Lobby game because the logic of moving pieces depends on it
        currentTurn = currentTurn == PlayerOne ? PlayerTwo : PlayerOne;

        Client temp = currentTurnClient;
        currentTurnClient = nonCurruentTurnClient;
        nonCurruentTurnClient = temp;

        std::wcout << "swapped turns, next game loop" << std::endl;

    }
}