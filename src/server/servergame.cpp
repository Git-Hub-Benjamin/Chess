#include "server.h"

// 0 Both responded
// 1 Only currTurnClient Responded
// 2 Only nonCurrTurnClient Responded
int Online_ChessGame::three_second_check_on_clients(std::string& str){
    bool client1_rdy = false;
    bool client2_rdy = false;
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    for(int i = 0; i < 3; i++){
        int byte_read = recv(current_turn_client.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            buffer[byte_read] = '\0';

            std::wcout << "Trying to find: " << convertString(str) << 
                        "\nVs            : " << convertString(std::string(buffer)) << std::endl;
            if(std::string(buffer).compare(str) == 0){
                std::wcout << "TurnClient sent equal string..." << std::endl;
                client1_rdy = true;
            }
        }

        byte_read = recv(non_current_turn_client.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            buffer[byte_read] = '\0';

            std::wcout << "Trying to find: " << convertString(str) << 
                        "\nVs            : " << convertString(std::string(buffer)) << std::endl;
            if(std::string(buffer).compare(str) == 0){
                std::wcout << "NonTurnClient sent equal string..." << std::endl;
                client2_rdy = true;
            }
        }

        if(client1_rdy && client2_rdy)
            break; // No point to keep checking

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if(client1_rdy && client2_rdy)
        return 0;
    else if(client1_rdy)
        return 1;
    else 
        return 2;
}

bool Online_ChessGame::end_turn_client_check_in(){
    std::string str(CLIENT_RDY_FOR_NEXT_TURN);
    return (!three_second_check_on_clients(str));
}

bool Online_ChessGame::send_preturn_check_in(std::string& msg){
    if(send(current_turn_client.Game.fd, (void*)msg.c_str(), msg.length(), 0) < 0 || send(non_current_turn_client.Game.fd, (void*)msg.c_str(), msg.length(), 0) < 0)
        return false;
    return true;
}

// -1 - Server fault
// 0 -- Good
// 1 -- Client fault
int Online_ChessGame::recieve_non_turn_client_check_in(){
    char buf[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(non_current_turn_client.Game.fd, (void*)buf, sizeof(buf), 0);

    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 1;
    
    return 0;
}

// -1 - Server fault
// 0 -- Good
// 1 -- Client fault
int Online_ChessGame::recieve_turn_client_move(std::string& out){
    char buf[ONLINE_BUFFER_SIZE] = {0};

    std::wcout << "Waiting for current turn client to send their move..." << std::endl;
    int byte_read = recv(current_turn_client.Game.fd, (void*)buf, sizeof(buf), 0); 

    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 1;

    buf[byte_read] = '\0';
    out = buf;
    return 0;
}

void Online_ChessGame::format_turn_move(std::string& in, std::wstring& move, std::wstring& to){
    move += in[CLIENT_MOVE_INDEX_AFTER_COLON];
    move += in[CLIENT_MOVE_INDEX_AFTER_COLON + 1];
    to +=   in[CLIENT_TO_INDEX_AFTER_COLON];
    to +=   in[CLIENT_TO_INDEX_AFTER_COLON + 1];
}

void Online_ChessGame::convert_move_square(GameSqaure*& movesquare, GameSqaure*& moveToSquare, std::wstring& move, std::wstring& to, bool in_check){
    if(in_check){
        // The client was automatcially put to move the king
        movesquare = Lobby_Game.KingPositions[current_turn_client.Player - 1];
    }else{
        movesquare = moveConverter(Lobby_Game, move);
    }

    moveToSquare = moveConverter(Lobby_Game, to);
}

void Online_ChessGame::check_valid_move(ChessGame& LobbyGame, GameSqaure* movePiece, GameSqaure* moveToSquare, std::string& out){
    out = SERVER_CLIENT_VERIFY_MOVE_VALID;

    if(!verifyMove(LobbyGame, *movePiece, *moveToSquare)){
        out = SERVER_CLIENT_VERIFY_MOVE_INVALID;
    }else{

        // Check if making this move puts them in check
        GameSqaure copyOfSquareBeingMoved = *movePiece;
        GameSqaure copyOfSquareBeingMovedTo = *moveToSquare;

        int res = makeMove(LobbyGame, *movePiece, *moveToSquare); //! CAUSING SEGMENTATION FAULT, Maybe something to do with copy? it was working fine before...
        if(res == 2){
            //! I dont know when makemove would return 2, but for later if i need it
        }else{
            if(!kingSafe(LobbyGame)){
                out = SERVER_CLIENT_VERIFY_MOVE_INVALID_CHECK;
                makeMove(LobbyGame, copyOfSquareBeingMoved, copyOfSquareBeingMovedTo); // Revert move
            }
        }
    }
}

// False --> End game, client connection went wrong
// True  --> Both clients connected continue
bool Online_ChessGame::verify_client_connection_init_turn(){

    std::string str(SERVER_CLIENT_ACK_MATCH_RDY);
    int res = three_second_check_on_clients(str);

    if(res == 0){
        
        // If both are ready, the next step is to send the pre turn check in
        return true;

    }else if(res == 1){
        // NonTurnClient fault
        std::wcout << "NonTurnClient Fault" << std::endl;
        send(current_turn_client.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(non_current_turn_client.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }else{
        // CurrentTurnClinet 1 fault
        std::wcout << "CurrTurnClinet Fault" << std::endl;
        send(non_current_turn_client.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(current_turn_client.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }
    return false;
}

void Online_ChessGame::gameloop(){
 
    // Make sure both clients send ("match-start")
    if(!verify_client_connection_init_turn()){
        std::wcout << "client connection was not successful" << std::endl;
        queue_lobby_close(lobby_status);
        return; 
    }

    while(!Lobby_Game.gameover){

        set_terminal_color(GREEN);
        std::wcout << "Current Turn Client: " << convertString(current_turn_client.CLIENT_STRING_ID) << std::endl;
        std::wcout << "Not Current Turn Client: " << convertString(non_current_turn_client.CLIENT_STRING_ID) << std::endl;
        set_terminal_color(DEFAULT);

        print_board(Lobby_Game);

        std::wcout << "Test to make print of gamesquare works in server" << std::endl;
        bool CURR_TURN_IN_CHECK = false;

        // Make sure to switch currentTurn at the end of turn as always
        std::string pre_turn_check_in;

        if(!kingSafe(Lobby_Game)){
            if(!checkMate(Lobby_Game)){ // Check checkmate for the current players turn
                pre_turn_check_in = GAMESTATUS_CHECK;
                CURR_TURN_IN_CHECK = true;
            }else{
                pre_turn_check_in = GAMESTATUS_CHECKMATE;
                Lobby_Game.gameover = true;
                break;
            }
        }else{
            pre_turn_check_in = GAMESTATUS_ALL_GOOD;
        }

        //! This is where we would do some checks for GAMESTATUS_DC

        std::wcout << "Server sending pre check in" << std::endl;
        if(!send_preturn_check_in(pre_turn_check_in)){
            // Tell clients there was a socket error, Server fault
            set_terminal_color(RED);
            std::wcout << "ERROR -- SOCKET ERROR" << std::endl;
            set_terminal_color(DEFAULT);
        }
        std::wcout << "Server sent pre turn check in" << std::endl;

        std::wcout << "Waiting for the non current turn client to send check in" << std::endl;
        int res = recieve_non_turn_client_check_in();
        if(res < 0){
            // Tell clients there was a socket error, Server fault
        }else if(res > 0){
            // Client fault, DC
        }
        std::wcout << "Non turn client sent their check in to the server" << std::endl;

        while(true){

            std::string response;
            res = recieve_turn_client_move(response);
            std::wcout << "Response sent from client: " << convertString(response) << std::endl; 
            if(res < 0){
                // Tell clients there was a socket error, Server fault
            }else if(res > 0){
                // Client fault, DC
            }

            std::wstring move;
            std::wstring to;
            format_turn_move(response, move, to);

            GameSqaure* movePiece;
            GameSqaure* moveToSquare;

            convert_move_square(movePiece, moveToSquare, move, to, CURR_TURN_IN_CHECK);

            std::wcout << "Client movesquare --> ";
            movePiece->print();
            std::wcout << "\nClient movetoSquare --> ";
            moveToSquare->print();
            std::wcout << std::endl;

            // Got move and to, now check valid
            std::string validaty_of_move_str;
            
            check_valid_move(Lobby_Game, movePiece, moveToSquare, validaty_of_move_str);

            if(send(current_turn_client.Game.fd, (void*)validaty_of_move_str.c_str(), validaty_of_move_str.length(), 0) < 0){
                // Tell clients there was a socket error, Server fault
            }

            // If its invalid, we have to do it all again
            if(validaty_of_move_str.compare(SERVER_CLIENT_VERIFY_MOVE_VALID) != 0)
                continue;

            // Valid move, the other player is still expecting the move the other player made,
            // Since we already did the checks on it we can just send it

            if(send(non_current_turn_client.Game.fd, (void*)response.c_str(), response.length(), 0) < 0){
                // Tell clients there was a socket error, Server fault
            }
            break;
        }

        if(!end_turn_client_check_in()){
            std::wcout << "Client connection died at end turn check." << std::endl;
            queue_lobby_close(lobby_status);
            return; // So self can join with main thread
        }

        // Swap turns
        Client& Temp = current_turn_client;
        current_turn_client = non_current_turn_client;
        non_current_turn_client = Temp;

    }
}