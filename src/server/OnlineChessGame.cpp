#include "server.hpp"

// False --> End game, client connection went wrong
// True  --> Both clients connected continue
//* Make sure both clients send ready for match before starting
bool OnlineChessGame::initClientConnection(){

    std::string str(SERVER_CLIENT_ACK_MATCH_RDY);
    int res = threeSecVerifyClientTimeout(str);

    if(res == 0){
        
        // If both are ready, the next step is to send the pre turn check in
        return true;

    }else if(res == 1){
        // NonTurnClient fault
        std::wcout << "NonTurnClient Fault" << std::endl;
        send(currentTurnClient.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(nonCurruentTurnClient.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }else{
        // CurrentTurnClinet 1 fault
        std::wcout << "CurrTurnClinet Fault" << std::endl;
        send(currentTurnClient.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(nonCurruentTurnClient.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }
    return false;
}

// 0 Both responded
// 1 Only currTurnClient Responded
// 2 Only nonCurrTurnClient Responded
int OnlineChessGame::threeSecVerifyClientTimeout(std::string& str){
    bool client1_rdy = false;
    bool client2_rdy = false;
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    for(int i = 0; i < 3; i++){
        int byte_read = recv(currentTurnClient.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
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

        byte_read = recv(nonCurruentTurnClient.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
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

//* Check in with both clients before continuing to the next turn
bool OnlineChessGame::endOfTurnClientsCheckIn(){
    std::string str(CLIENT_RDY_FOR_NEXT_TURN);
    return (!threeSecVerifyClientTimeout(str));
}

//* Sending the current gamestatus of the game to the clients
bool OnlineChessGame::sendPreTurnCheckIn(std::string& msg){
    if(send(currentTurnClient.Game.fd, (void*)msg.c_str(), msg.length(), 0) < 0 || send(nonCurruentTurnClient.Game.fd, (void*)msg.c_str(), msg.length(), 0) < 0)
        return false;
    return true;
}

// -1 - Server fault
// 0 -- Good
// 1 -- Client fault
//* To adhere to client -> server -> client communication we need to have one more
//* communication between the non turn client and the server before recieving from the turn client
int OnlineChessGame::nonTurnSpecificClientCheckIn(){
    char buf[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(nonCurruentTurnClient.Game.fd, (void*)buf, sizeof(buf), 0);

    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 1;
    
    return 0;
}

// -1 - Server fault
// 0 -- Good
// 1 -- Client fault
//* Recieve the current turn clients move
int OnlineChessGame::recieveCurrTurnMove(std::string& out){
    char buf[ONLINE_BUFFER_SIZE] = {0};

    std::wcout << "Waiting for current turn client to send their move..." << std::endl;
    int byte_read = recv(currentTurnClient.Game.fd, (void*)buf, sizeof(buf), 0); 

    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 1;

    buf[byte_read] = '\0';
    out = buf;
    return 0;
}


