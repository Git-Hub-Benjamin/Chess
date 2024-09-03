#include "server.hpp"

//! Handling send fails, the good thing about send fails is that the program is till running
//! if it fails it could be for many reasons, we tried to send a fail message to the client but it failed
//! the best we can do is just close the client connection on our end and continue processing,
//! we dont want to bring down the entire server because of one fail, maybe log the fail and the infomation surrounding it
//! set the client connection to be closed and continue...


bool OnlineChessGame::sendCurrTurnValidatityOfMove(std::string& msg) {
    if (!checkClientConnection())
        return false;
        
    send(nonCurruentTurnClient.gameFd, (void*)msg.c_str(), msg.length(), 0);
    // Tell clients there was a socket error, Server fault
    
    return true;
}

void OnlineChessGame::disconnectClient(Client& client) {
    //! IDK HOW TO HANDLE IF SEND FAILS!
    send(client.gameFd, GAMESTATUS_GAMEOVER_DC, sizeof(GAMESTATUS_GAMEOVER_DC), 0);
    client.status = DISCONNECTED_KILL; // Tell main thread it is ok to kill this
    close(client.gameFd);
}

// To be called when a client DC and this will be called for the other client that is still active
void OnlineChessGame::endGameForClient(Client& client, std::string msg) {
    //! IDK HOW TO HANDLE IF SEND FAILS!
    if (send(client.gameFd, msg.c_str(), msg.length(), 0) < 0) {
        disconnectClient(client);
    }
    client.status = WAITING; // Send back to waiting area
}

// To be called before ANY communication between any game socket in game
// True BOTH clients are still connected,
// False BOTH or ONE of the clients DC, End game
bool OnlineChessGame::checkClientConnection() {

    if (currentTurnClient.status == DISCONNECTED && nonCurruentTurnClient.status == DISCONNECTED) {
        disconnectClient(currentTurnClient);
        disconnectClient(nonCurruentTurnClient);
        return false;
    } else if (currentTurnClient.status == DISCONNECTED) {
        disconnectClient(currentTurnClient);
        endGameForClient(nonCurruentTurnClient, GAMESTATUS_GAMEOVER_DC);
        return false;
    } else if (nonCurruentTurnClient.status == DISCONNECTED) {
        disconnectClient(nonCurruentTurnClient);
        endGameForClient(currentTurnClient, GAMESTATUS_GAMEOVER_DC);
        return false;
    }

    return true;
}

void OnlineChessGame::closeSelfLobby(){
    LobbyStatus = QUEUE_KILL;
}

// False --> End game, client connection went wrong
// True  --> Both clients connected continue
//* Make sure both clients send ready for match before starting
bool OnlineChessGame::initClientConnection(){

    if (!checkClientConnection()) {
        closeSelfLobby();
        return false;
    }

    std::string str(SERVER_CLIENT_ACK_MATCH_RDY);
    int res = threeSecVerifyClientTimeout(str);

    if(res == 0){
        
        // If both are ready, the next step is to send the pre turn check in
        return true;

    }else if(res == 1){
        // NonTurnClient fault
        std::wcout << "NonTurnClient Fault" << std::endl;
        send(currentTurnClient.gameFd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(nonCurruentTurnClient.gameFd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }else{
        // CurrentTurnClinet 1 fault
        std::wcout << "CurrTurnClinet Fault" << std::endl;
        send(currentTurnClient.gameFd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(nonCurruentTurnClient.gameFd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
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
        int byte_read = recv(currentTurnClient.gameFd, buffer, sizeof(buffer), MSG_DONTWAIT);
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

        byte_read = recv(nonCurruentTurnClient.gameFd, buffer, sizeof(buffer), MSG_DONTWAIT);
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

    if (!checkClientConnection())
        return false;

    std::string str(CLIENT_RDY_FOR_NEXT_TURN);
    return (!threeSecVerifyClientTimeout(str));
}

//* Sending the current gamestatus of the game to the clients
bool OnlineChessGame::sendPreTurnCheckIn(std::string& msg){
    if (!checkClientConnection())
        return false;

    //! Handle Error sending!
    send(currentTurnClient.gameFd, (void*)msg.c_str(), msg.length(), 0);
    send(nonCurruentTurnClient.gameFd, (void*)msg.c_str(), msg.length(), 0);

    return true;
}

// -1 - Server fault
// 0 - Client fault
// 1 -- Good
//* To adhere to client -> server -> client communication we need to have one more
//* communication between the non turn client and the server before recieving from the turn client
int OnlineChessGame::nonTurnSpecificClientCheckIn(){
    if (!checkClientConnection())
        return 0; // Same as False

    char buf[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(nonCurruentTurnClient.gameFd, (void*)buf, sizeof(buf), 0);

    //! HANDLE
    if(byte_read < 0)
        return -1; 

    if(byte_read <= 0)
        return 1;
    
    return 1;
}

// -1 - Server fault
// 0 -- Client fault, end game
// 1 -- Good
//* Recieve the current turn clients move
int OnlineChessGame::recieveCurrTurnMove(std::string& out){
    if (!checkClientConnection())
        return 0; // Same as False

    char buf[ONLINE_BUFFER_SIZE] = {0};

    std::wcout << "Waiting for current turn client to send their move..." << std::endl;
    int byte_read = recv(currentTurnClient.gameFd, (void*)buf, sizeof(buf), 0); 


    //! HANDLE
    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 0;

    buf[byte_read] = '\0';
    out = buf;
    return 1;
}


