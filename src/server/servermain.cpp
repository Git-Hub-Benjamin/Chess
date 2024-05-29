#include "../chess.hpp"
#include "./socket/socketutil.h"
#include "./server-terminal-communication/servercommands.hpp"
#include <stdio.h>
#include <string>
#include <pthread.h> // may user thread
#include <thread>
#include <mutex>
#include <time.h>
#include <unistd.h>
#include <random>
#include "server.h"
#include <netinet/in.h>
#include <fcntl.h>
#include <unordered_map>
#include <memory>
#include <csignal>
#include <execinfo.h>
#include <atomic>
#include <sys/stat.h>


#define MAX_CLIENT 8

std::mutex mtx;
static Client* clients[MAX_CLIENT] = { nullptr };
static Online_ChessGame* Client_Lobbies[MAX_CLIENT / 2] = { nullptr };
static std::vector<int> queueLobbiesToDelete;
static std::unordered_map<std::string, Client*> PRIVATE_LOBBY_CODES;
std::atomic_bool KILL_SERVER(false);

std::string charP_to_string(const char* str){
    std::string temp = str;
    return std::string(temp.begin(), temp.end());
}

std::wstring string_to_wstring(std::string &str){
    return std::wstring(str.begin(), str.end());
}

void lobby_access_lock() { mtx.lock(); }
void lobby_access_unlock() { mtx.unlock(); }


void close_client(int index){

    std::string code_to_erase;

    // see if the client had any private room codes that we need to close
    for(auto [code, client]: PRIVATE_LOBBY_CODES){

        if(client->Game.index != index)
            continue;

        // We dont need to keep checking, client should only be able to open 1 at a time        
        code_to_erase = code;
        break; 
    }

    PRIVATE_LOBBY_CODES.erase(code_to_erase);

    if(clients[index] != nullptr)
        delete clients[index];

    clients[index] = nullptr; // since a lot of things check against nullptr
    
}

int add_client(int client_fd, std::string& bind_msg){

    int res = -1;
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] == nullptr){
            // found an open position
            clients[client_index] = new Client(client_fd, client_index, bind_msg);
            res = 0;

            std::wcout << "Made a new client with this bind string id: " << convertString(bind_msg) << std::endl;
            break; // need to unlock!
        }
    }

    return res;
}

int waiting_players_count(){
    int count = 0;
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] != nullptr && clients[client_index]->status == WAITING)
            count++;
    }
    return count;
}

int playing_players_count(){
    int count = 0;
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] != nullptr && clients[client_index]->status == PLAYING)
            count++;
    }
    return count;
}

//! Send the poll message at 0 seconds, wait 10 seconds then run a different function
//! that recieves the poll message and does the same thing as poll_for_active_clients but split it in 2
void send_poll_on_active_clients(std::vector<Client*>& check_active){
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        // Make sure client is in slot and bound
        if(clients[client_index] != nullptr && clients[client_index]->status != UNBOUND)
            check_active.push_back(clients[client_index]);
    }

    for(Client* client: check_active){
        // Maybe make this non blocking
        send(client->Poll.fd, (void*)SERVER_POLL_MSG, sizeof(SERVER_POLL_MSG), 0); 
    }

    int count = playing_players_count() + waiting_players_count();
    std::wcout << "Sending poll msg to " << count << " clients" << std::endl;
}


//! only close clients from here
void recieve_poll_on_active_clients(std::vector<Client*>& check_active){
     char buffer[ONLINE_BUFFER_SIZE] = {0};
    for(Client* client: check_active){
        int bytes_recieved = recv(client->Poll.fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
        buffer[bytes_recieved] = '\0';

        std::string msg_from_client(buffer);

        if(bytes_recieved <= 0)
            close_client(client->Poll.index);
        
        if(std::string(buffer).compare(CLIENT_POLL_MSG) != 0)
            close_client(client->Poll.index);  
    }

    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] != nullptr && clients[client_index]->status == UNBOUND){
            // Clear any clients that may be stuck on UNBOUND
            // If greater than 3 seconds, close client
            if(3 < std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - clients[client_index]->timeSinceUnbound).count())
                close_client(clients[client_index]->Poll.index);
        }
    }
}



Client* bind_on_client_str(std::string str){
    Client* res = nullptr;
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] == nullptr)
            continue;
        if(clients[client_index]->status != UNBOUND)
            continue;

        if(clients[client_index]->CLIENT_STRING_ID.compare(str) != 0)
            continue;
        // Found match, return the current index pointer
        res = clients[client_index];
        break;
    }
    return res;
}

void accept_game_socket(int game_sock_fd){
    struct sockaddr_in* clientAddress;
    socklen_t client_addr_len = sizeof(*clientAddress);
    int clientSocketFD;

    while ((clientSocketFD = accept(game_sock_fd, (struct sockaddr*)&clientAddress, (socklen_t*)&client_addr_len)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No pending connections available, continue the loop
            return;
        } else if (errno != EINTR) {
            std::wcout << "Game socket, Accept failed: " << std::strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
    }


    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int byte_recieved = recv(clientSocketFD, (void*)buffer, sizeof(buffer), 0);
    if(byte_recieved <= 0){
        // Not sure when this would happen
        send(clientSocketFD, (void*)SERVER_CLIENT_INIT_FAIL, sizeof(SERVER_CLIENT_INIT_FAIL), 0);
        return;
    }

    buffer[byte_recieved] = '\0';
    std::string client_bind_id(buffer);
    Client* found_unbound_client = bind_on_client_str(client_bind_id.substr(BIND_INDEX_OF_START_RAND_STRING, byte_recieved - 1));
    
    if(found_unbound_client == nullptr){
        // means that somehow the first socket connected to the server, then 10 whole seconds went by
        // and the poll_for_active_clients() function kicked the inactive first socket because it stopped responding
        // but somehow this socket still tried to connect, in reality this would never happen unless,
        // you force a connect to this address and port 
        std::wcout << "Could not find a matching client with this random string" << std::endl;
        send(clientSocketFD, (void*)SERVER_CLIENT_INIT_FAIL, sizeof(SERVER_CLIENT_INIT_FAIL), 0);
        return;
    }

    // Send success response to client
    send(clientSocketFD, (void*)SERVER_CLIENT_INIT_GOOD, sizeof(SERVER_CLIENT_INIT_GOOD), 0);

    found_unbound_client->Game.fd = clientSocketFD;
    // Not neccessary but why not
    found_unbound_client->Game.index = found_unbound_client->Poll.index; 

    //? Also im not sure if we need to client access lock here? I dont think
    //? We need to becasue we are changing unbound->waiting and nothing is checking for that
    // Up to waiting so it can be placed into a game
    found_unbound_client->status = WAITING;
}

void handle_client_none_selected_game_move(Client& client){
    // Going to send the 
}

void clear_old_private_codes(Client& client){
    std::string old_code;
    for(auto [code, cli]: PRIVATE_LOBBY_CODES){

        if(&client != &(*cli))
            continue;

        // should only be one code at a time
        PRIVATE_LOBBY_CODES.erase(code);
        break; 
    }
}


std::string create_private_room_with_code(Client& client_creating_room){
    // Might be overkill to have to do this
    clear_old_private_codes(client_creating_room);
    std::string rand_code = generateRandomString(5);
    PRIVATE_LOBBY_CODES[rand_code] = &client_creating_room;
    return rand_code;
}

Client* check_private_room_with_code(std::string code){
    if(PRIVATE_LOBBY_CODES.find(code) != PRIVATE_LOBBY_CODES.end())
        return &(*PRIVATE_LOBBY_CODES[code]);
    return nullptr;
}

//! VERIFY CHECKING ADDRESSES IS WHAT I THINK IT IS
void close_private_room_associated_with_client(Client& client){
    for(auto& [key, cli]: PRIVATE_LOBBY_CODES){
        if(&client == &(*cli)){
            PRIVATE_LOBBY_CODES.erase(key);
            break;
        }
    }
}

bool both_clients_joined_successfully(Client& client1, Client& client2){
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int byte_recieved = recv(client1.Game.fd, (void*)buffer, sizeof(buffer), 0);
    
    if(byte_recieved <= 0){
        // HANDLE
        send(client2.Game.fd, (void*)GAME_FAIL_OTHER_PLAYER_DC, sizeof(GAME_FAIL_OTHER_PLAYER_DC), 0);
        return false;
    }
    
    byte_recieved = recv(client2.Game.fd, (void*)buffer, sizeof(buffer), 0);
    if(byte_recieved <= 0){
        // HANDLE
        send(client1.Game.fd, (void*)GAME_FAIL_OTHER_PLAYER_DC, sizeof(GAME_FAIL_OTHER_PLAYER_DC), 0);
        return false;
    }

    return true;
}

enum Owner fifty_fifty_player_turn(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, 1);
    if(distribution(gen) + 1 == 1){ // Client 1 will be PONE, meaning will go first
        return PONE;
    }
    return PTWO;
}

// False --> End game, client connection went wrong
// True  --> Both clients connected continue
bool verify_client_connection_init_turn(int index){
    // Give each 3 seconds to send back match-rdy msg

    bool client1_rdy = false;
    bool client2_rdy = false;
    Client& client1 = Client_Lobbies[index]->client1;
    Client& client2 = Client_Lobbies[index]->client2;
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    for(int i = 0; i < 3; i++){
        int byte_read = recv(client1.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            client1_rdy = true;
        }

        byte_read = recv(client2.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            client2_rdy = true;
        }

        if(client1_rdy && client2_rdy)
            break; // No point to keep checking

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::wcout << "client 1 rdy? --> " << (client1_rdy ? "True" : "False") << std::endl;
    std::wcout << "client 2 rdy? --> " << (client2_rdy ? "True" : "False") << std::endl;

    if(client1_rdy && client2_rdy){
        
        // If both are ready, the next step is to send the pre turn check in
        return true;

    }else if(client1_rdy){
        // Client 2 fault
        std::wcout << "Client2 Fault --> " << convertString(client2.CLIENT_STRING_ID) << std::endl;
        send(client1.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(client2.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }else{
        // Client 1 fault
        std::wcout << "Client1 Fault --> " << convertString(client1.CLIENT_STRING_ID) << std::endl;
        send(client2.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT), 0);
        send(client1.Game.fd, (void*)SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT, sizeof(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT), 0);
    }
    return false;
}

void delete_lobby(int index){
    delete Client_Lobbies[index];
}

void queue_lobby_close(enum LOBBY_STATUS& stat){
    while(stat != AKN_KILL){
        std::this_thread::yield();
    }
    // Very stubbord about this but since the main thread will be setting AKN_KILL
    // and Joining this thread with the main thread there is a small change that the 
    // main thread will set AKN_KILL but then this thread gets scheduled meaning that
    // the main thread never is able to set this thread to join
}

bool begin_game_client_check(int lobby_index){
    bool client1_rdy = false;
    bool client2_rdy = false;
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    Client& client1 = Client_Lobbies[lobby_index]->client1;
    Client& client2 = Client_Lobbies[lobby_index]->client2;

    for(int i = 0; i < 3; i++){
        int byte_read = recv(client1.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            client1_rdy = true;
        }

        byte_read = recv(client2.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            client2_rdy = true;
        }

        if(client1_rdy && client2_rdy)
            break; // No point to keep checking

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::wcout << "Begin Game client verify, Client 1: " << (client1_rdy == true ? "True" : "False") << std::endl;
    std::wcout << "Begin Game client verify, Client 2: " << (client2_rdy == true ? "True" : "False") << std::endl;


    return (client1_rdy && client2_rdy);
}

bool end_turn_client_check_in(int lobby_index){
    bool client1_rdy = false;
    bool client2_rdy = false;
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    Client& client1 = Client_Lobbies[lobby_index]->client1;
    Client& client2 = Client_Lobbies[lobby_index]->client2;

    for(int i = 0; i < 3; i++){
        int byte_read = recv(client1.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            client1_rdy = true;
        }

        byte_read = recv(client2.Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if(byte_read > 0){
            // We dont have to check, if there is more than 0 bytes being sent it means they are ready
            client2_rdy = true;
        }

        if(client1_rdy && client2_rdy)
            break; // No point to keep checking

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return (client1_rdy && client2_rdy);
}

bool send_preturn_check_in(int index, std::string& msg){
    if(send(Client_Lobbies[index]->client1.Game.fd, (void*)msg.c_str(), msg.length(), 0) < 0 || send(Client_Lobbies[index]->client2.Game.fd, (void*)msg.c_str(), msg.length(), 0) < 0)
        return false;
    return true;
}

// -1 - Server fault
// 0 -- Good
// 1 -- Client fault
int recieve_non_turn_client_check_in(Client& nonTurnClient){
    char buf[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(nonTurnClient.Game.fd, (void*)buf, sizeof(buf), 0);

    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 1;
    
    return 0;
}

// -1 - Server fault
// 0 -- Good
// 1 -- Client fault
int recieve_turn_client_move(Client& turnClient, std::string& out){
    char buf[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(turnClient.Game.fd, (void*)buf, sizeof(buf), 0); 

    if(byte_read < 0)
        return -1;

    if(byte_read <= 0)
        return 1;

    buf[byte_read] = '\0';
    out = buf;
    return 0;
}

void format_turn_move(std::string& in, std::wstring& move, std::wstring& to){
    move += in[CLIENT_MOVE_INDEX_AFTER_COLON];
    move += in[CLIENT_MOVE_INDEX_AFTER_COLON + 1];
    to +=   in[CLIENT_TO_INDEX_AFTER_COLON];
    to +=   in[CLIENT_TO_INDEX_AFTER_COLON + 1];
}

void convert_move_square(ChessGame& LobbyGame, Client& currTurnClient, GameSqaure* movesquare, GameSqaure* moveToSquare, std::wstring& move, std::wstring& to){
    if(move.compare(L"KI") == 0){
        // The client was automatcially put to move the king
        movesquare = LobbyGame.KingPositions[currTurnClient.Player - 1];
    }else{
        movesquare = moveConverter(LobbyGame, move);
    }

    moveToSquare = moveConverter(LobbyGame, to);
}

void check_valid_move(ChessGame& LobbyGame, GameSqaure* movePiece, GameSqaure* moveToSquare, std::string& out){
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

void GAME_FOR_TWO_CLIENTS(int lobbies_index){

    // Init these variables
    Online_ChessGame& Lobby = *Client_Lobbies[lobbies_index];    
    ChessGame& Lobby_Game = Lobby.game;

    Client& currTurnClient = (Lobby_Game.currentTurn == Lobby.client1.Player ? Lobby.client1 : Lobby.client2);
    Client& notTurnClient  = (&currTurnClient == &Lobby.client1 ? Lobby.client2 : Lobby.client1);

    // Make sure both clients send ("match-start")
    if(!verify_client_connection_init_turn(lobbies_index)){
        std::wcout << "client connection was not successful" << std::endl;
        queue_lobby_close(Lobby.lobby_status);
        return; 
    }

    while(!Lobby_Game.gameover){

        // Make sure to switch currentTurn at the end of turn as always
        std::string pre_turn_check_in;

        if(!kingSafe(Lobby_Game)){
            if(!checkMate(Lobby_Game)){ // Check checkmate for the current players turn
                pre_turn_check_in = GAMESTATUS_CHECK;
            }else{
                pre_turn_check_in = GAMESTATUS_CHECKMATE;
                Lobby_Game.gameover = true;
                break;
            }
        }else{
            pre_turn_check_in = GAMESTATUS_ALL_GOOD;
        }

        //! This is where we would do some checks for GAMESTATUS_DC

        if(!send_preturn_check_in(lobbies_index, pre_turn_check_in)){
            // Tell clients there was a socket error, Server fault
        }

        int res = recieve_non_turn_client_check_in(notTurnClient);
        if(res < 0){
            // Tell clients there was a socket error, Server fault
        }else if(res > 0){
            // Client fault, DC
        }

        while(true){

            std::string response;
            res = recieve_turn_client_move(currTurnClient, response);
            if(res < 0){
                // Tell clients there was a socket error, Server fault
            }else if(res > 0){
                // Client fault, DC
            }

            std::wstring move;
            std::wstring to;
            format_turn_move(response, move, to);

            std::wcout << "\n\nClient move: " << move << std::endl;
            std::wcout << "\n\nClient to  : " <<  to  << std::endl;

            GameSqaure* movePiece;
            GameSqaure* moveToSquare;

            convert_move_square(Lobby_Game, currTurnClient, movePiece, moveToSquare, move, to);

            std::wcout << "Client movesquare --> ";
            movePiece->print();
            std::wcout << "\nClient movetoSquare --> ";
            moveToSquare->print();
            std::wcout << std::endl;

            // Got move and to, now check valid
            std::string validaty_of_move_str;
            
            check_valid_move(Lobby_Game, movePiece, moveToSquare, validaty_of_move_str);

            if(send(currTurnClient.Game.fd, (void*)validaty_of_move_str.c_str(), validaty_of_move_str.length(), 0) < 0){
                // Tell clients there was a socket error, Server fault
            }

            // If its invalid, we have to do it all again
            if(validaty_of_move_str.compare(SERVER_CLIENT_VERIFY_MOVE_VALID) != 0)
                continue;

            // Valid move, the other player is still expecting the move the other player made,
            // Since we already did the checks on it we can just send it

            send(notTurnClient.Game.fd, (void*)response.c_str(), response.length(), 0);
            break;
        }

        if(!end_turn_client_check_in(lobbies_index)){
            std::wcout << "Client connection died at end turn check." << std::endl;
            queue_lobby_close(Lobby.lobby_status);
            return; // So self can join with main thread
        }

        std::wcout << "Before switch, curr: " << convertString(currTurnClient.CLIENT_STRING_ID) << ", not curr: " << convertString(notTurnClient.CLIENT_STRING_ID) << std::endl;

        // Swap turns
        Client& Temp = currTurnClient;
        currTurnClient = notTurnClient;
        notTurnClient = Temp;

        std::wcout << "After switch, curr: " << convertString(currTurnClient.CLIENT_STRING_ID) << ", not curr: " << convertString(notTurnClient.CLIENT_STRING_ID) << std::endl;

    }
}

void schedule_rand_queue_clients(std::vector<std::pair<Client&, Client&>>& vec){
    int ready_clients = 0;
    int ready_client_one_index = -1;
    int ready_client_two_index = -1;

    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        
        if(clients[client_index] == nullptr) // No nullptrs
            continue;

        if(clients[client_index]->status != WAITING) // Only waiting clients
            continue;

        if(clients[client_index]->client_selected_game_move != QUEUE_RANDOM) // Only queue random game mode selected clients
            continue;
        
        // found waiting client, increase count
        ready_clients++;
        if(ready_client_one_index < 0)
            ready_client_one_index = client_index;
        else
            ready_client_two_index = client_index;

        if(ready_clients == 2){
            // send these two clients to a game
            vec.push_back({*clients[ready_client_one_index], *clients[ready_client_two_index]});
            ready_clients = 0;
            ready_client_one_index = -1;
            ready_client_two_index = -1;
        }
    }
}

//* IN CLIENT ACCESS LOCK
// assume not nullptr and the curr_client->status is waiting
std::string client_update_game_mode(Client* curr_client, std::vector<std::pair<Client&, Client&>>& vec) {
    std::string build_string = "";
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    // for every waiting client we are going to send the current active players
    // it is up to the client if they want to display / use the information

    // server activity to clients
    int byte_recieved = recv(curr_client->Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
    
    std::wcout << "Curr client: " << convertString(curr_client->CLIENT_STRING_ID) << ", bytes: " << byte_recieved  << std::endl;

    // Means that the client did not choose a gamemode yet, or client connection closed
    if (byte_recieved <= 0)
        return build_string;

    // New update on the client side, maybe they changed to random queue
    buffer[byte_recieved] = '\0';
    std::string msg_from_client(buffer);

    std::wcout << "=====\nMsg from " << convertString(curr_client->CLIENT_STRING_ID) << ": " << convertString(msg_from_client) << std::endl; 

    // The client could be doing the following...

    // 1. Closing their Game Socket FD, We need to clean up the Client instance and the poll socket associated with it
    // 2. Joining the random queue
    // 3. Leaving the random queue
    // 4. Creating a private lobby
    // 5. Closing a private lobby
    // 6. Joining a private lobby


    if(byte_recieved == 0){
        // Handling 1.
        // Means the client closed the connection
        //! Check that there is no problem with closing a client connection here
        //! Also, because i know before I said it was only going to be done from 
        //! the poll socket
    }else if(msg_from_client.compare(CLIENT_JOIN_RANDOM_QUEUE) == 0){
        // Handling 2. joining random queue
        build_string += SERVER_CLIENT_JOIN_RAND_QUEUE_GOOD;
        curr_client->client_selected_game_move = QUEUE_RANDOM;
    }else if(msg_from_client.compare(CLIENT_LEAVE_RANDOM_QUEUE) == 0){
        // Handling 3. leaving random queue
        build_string += SERVER_CLIENT_LEAVE_RAND_QUEUE_GOOD;
        curr_client->client_selected_game_move = NONE_SELECTED;
    }else if(msg_from_client.compare(CLIENT_CREATE_PRIVATE_LOBBY) == 0){
        // Handling 4. creating a private lobby
        build_string += SERVER_CREATE_LOBBY_GOOD;
        build_string += create_private_room_with_code(*curr_client);
    }else if(msg_from_client.compare(CLIENT_CLOSE_LOBBY) == 0){
        // Handling 5. closing a private lobby
        build_string += SERVER_CLIENT_CLOSE_LOBBY_GOOD;
        close_private_room_associated_with_client(*curr_client);
    }else{
        // Handling 6. joining private lobby
        Client* maker_of_lobby_client = check_private_room_with_code(msg_from_client.substr(INDEX_AFTER_COLON_JOIN_CODE, byte_recieved - 1));
        if (maker_of_lobby_client == nullptr) {
            build_string += SERVER_CLIENT_JOIN_PRIVATE_LOBBY_BAD; // invalid code, or room is closed now
        } else {
            std::wcout << "Scheduling these two clients, Maker: " << convertString(maker_of_lobby_client->CLIENT_STRING_ID) << ", Joiner: " << convertString(curr_client->CLIENT_STRING_ID) << std::endl;
            // Schedule two clients
            // We dont have to do build_string += MATCH_FOUND because the dispatcher will do that 
            vec.push_back({*maker_of_lobby_client, *curr_client});
        }
    }

    std::wcout << "Sending: " << convertString(build_string) << ", to: " << convertString(curr_client->CLIENT_STRING_ID) << "\n=====" << std::endl;

    return build_string;
}

std::string get_waiting_updates(){
    std::string waiting_clients_msg(WAITING_MSG_W); 
    waiting_clients_msg += std::to_string(waiting_players_count());
    waiting_clients_msg += std::string(WAITING_MSG_P);
    waiting_clients_msg += std::to_string(playing_players_count());
    return waiting_clients_msg;
}

//** IN ACCESS LOCK, SAFE TO DO THIS
void client_waiting_to_playing(Client& curr){
    curr.status = PLAYING;
    curr.RES_TO_CLIENT_WAITING = MATCH_FOUND;
}

void cleanup_inactive_lobbies(){
    for(int client_lobbies_index = 0; client_lobbies_index < MAX_CLIENT / 2; client_lobbies_index++){
        if(Client_Lobbies[client_lobbies_index] != nullptr && Client_Lobbies[client_lobbies_index]->lobby_status == QUEUE_KILL){
            // Looking for valid lobbies and ones that are queued to be killed by the main thread

        }
    }
    
}

#ifdef SERVER_TERMINAL
static int server_terminal_communication_fd;

static void kill_server(){}

static void display_client_info(Client& curr){
    std::wcout << "\n= = = = = = =" << std::endl;
    std::wcout << "Client string: " << convertString(curr.CLIENT_STRING_ID) << ", \nFile descriptor (Game/Poll): " << curr.Game.fd << "/" << curr.Poll.fd << ", \nIndex: " << curr.Game.index << ", \nStatus: " << (curr.status == UNBOUND ? L"Unbound" : curr.status == WAITING ? L"Waiting" : L"Playing") << std::endl;
    std::wcout << "= = = = = = =" << std::endl;
}

static void display_type_clients(enum CLIENT_STATUS status){
    int count = 0;
    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        if(clients[client_index] != nullptr){
            // IF UNBOUND IS PASSED THEN JUST DISPLAY ALL CLIENTS
            if(status == UNBOUND){
                display_client_info(*clients[client_index]);
                count++;
            }else if(clients[client_index]->status == status){
                display_client_info(*clients[client_index]);
                count++;
            }
        } 
    }

    set_terminal_color(DEFAULT);
    if(count == 0)
        if(status != UNBOUND)
            std::wcout << "No clients found with " << (status == WAITING ? "\"Waiting\"" : "\"Playing\"") << " status" << std::endl;
        else 
            std::wcout << "No clients on the server..." << std::endl;
    else
        if(status != UNBOUND)
            std::wcout << count << " client" << (count == 1 ? "" : "s") << " found with " << (status == WAITING ? "\"Waiting\"" : "\"Playing\"") << " status" << std::endl;
        else
            std::wcout << count << " client" << (count == 1 ? "" : "s") << " found on the server..." << std::endl;
    set_terminal_color(THIN);
}

static void display_game_lobbies(){
    std::wcout << "Unimplemented..." << std::endl;
}

static void display_private_lobbies_and_owner(){
    int count = 0;
    for(auto [code, player]: PRIVATE_LOBBY_CODES){
        std::wcout << "\n- - - - - - -" << std::endl;
        std::wcout << "Private lobby code: " << convertString(code) << "\nOwner: ";
        display_client_info(*player);
        std::wcout << "- - - - - - -" << std::endl;
        count++;
    } 
    set_terminal_color(DEFAULT);
    if(count == 0)
        std::wcout << "No client private lobbies found..." << std::endl;
    else
        std::wcout << count << " client private lobbies found..." << std::endl;
    set_terminal_color(THIN);
}

void handle_command(std::string command){

    if(command.compare(KILL_SERVER_CMD) == 0)
        kill_server(); // Kill server 
    else if(command.compare(ALL_CLIENTS_CMD) == 0 || command.compare(ALL_CLIENT_CMD) == 0)
        display_type_clients(UNBOUND); // Display all clients --> USING UNBOUND TO DISPLAY ALL
    else if(command.compare(WAITING_CLIENTS_CMD) == 0 || command.compare(WAIT_CLIENT_CMD) == 0)
        display_type_clients(WAITING); // Display all waiting clients
    else if(command.compare(PLAYING_CLIENTS_CMD) == 0 || command.compare(PLAY_CLIENT_CMD) == 0)
        display_type_clients(PLAYING); // Display all playing clients
    else if(command.compare(CLIENT_LOBBIES_CMD) == 0 || command.compare(CLIENT_LOBBY_CMD) == 0)
        display_game_lobbies(); // Display all client game lobbies
    else if(command.compare(CLIENT_PRIVATE_LOBBIES_CMD) == 0 || command.compare(CLIENT_PRIVATE_LOBBY_CMD) == 0)
        display_private_lobbies_and_owner(); // Display all private lobbies and their creator / maker
    else if(command.compare(SERVER_KILL_LOBBIES_AND_THREAD) == 0 || command.compare(KILL_LOBBY_THREAD_CMD) == 0)
        cleanup_inactive_lobbies(); // this will not work as expected
    else
        std::wcout << "Unknown command --> " << convertString(command) << std::endl;
}

void read_fifo(){
    char buffer[ONLINE_BUFFER_SIZE];
    ssize_t bytesRead;

    bytesRead = read(server_terminal_communication_fd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the buffer
        handle_command(std::string(buffer));
    } else if (bytesRead == -1 && errno == EAGAIN) {
        // No data available, perform other tasks in the thread
        // (e.g., process other events, perform calculations)
        return;
    }
}
#endif

// -1 No open lobby
// Anything else is the index into client_lobbies to use
int check_for_open_lobby(){
    for(int client_lobbies_index = 0; client_lobbies_index < MAX_CLIENT / 2; client_lobbies_index++){
        if(Client_Lobbies[client_lobbies_index] == nullptr) // looking for a nullptr
            return client_lobbies_index;
    }
    return -1;
}

void initalize_client_lobby(std::thread* game_thread, int lobby_index, Client& client1, Client& client2){
    Client_Lobbies[lobby_index] = new Online_ChessGame(game_thread, lobby_index, client1, client2);
}

// Runs random queue dispatcher 1 time a second, and sends updates to 
// waiting clients 1 time every 3 seconds
void dispatch_and_send_waiting_updates(){
    
    //std::string server_activity_msg = get_waiting_updates();
    std::vector<std::pair<Client&, Client&>> CLIENTS_TO_DISPATCH;
    std::vector<int> activeIndexs;

    for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
        
        Client* curr_client = clients[client_index];
        
        if(curr_client == nullptr || curr_client->status != WAITING)
            continue;

        activeIndexs.push_back(client_index);

        // Send waiting / active players online to WAITING clients
        std::string response_to_client; //= server_activity_msg;

        response_to_client += client_update_game_mode(curr_client, CLIENTS_TO_DISPATCH);

        curr_client->RES_TO_CLIENT_WAITING = response_to_client;

    }

    schedule_rand_queue_clients(CLIENTS_TO_DISPATCH); 

    // Schedule matches found
    for(auto pair: CLIENTS_TO_DISPATCH){

        std::wcout << "= = = = =\n\nFOUND MATCH FOR TWO CLIENTS\nClient 1: " << convertString(pair.first.CLIENT_STRING_ID) << "\nClient 2: " << convertString(pair.second.CLIENT_STRING_ID) << "\n\n= = = = =" << std::endl;
        int lobby_index = check_for_open_lobby();

        if(lobby_index < 0){
            // Means no open lobby was found
            pair.first.RES_TO_CLIENT_WAITING = SERVER_CLIENT_MAX_CAPACITY;
            pair.second.RES_TO_CLIENT_WAITING = SERVER_CLIENT_MAX_CAPACITY;

            set_terminal_color(RED);
            std::wcout << "Nevermind... No lobbies available..." << std::endl;
            set_terminal_color(DEFAULT);

            continue;
        }

        enum Owner playerSelect = fifty_fifty_player_turn();

        pair.first.Player = playerSelect;
        pair.second.Player = (playerSelect == PONE ? PTWO : PONE);;

        std::string client1_res = MATCH_FOUND;
        client1_res += ",opponent:";
        std::string client2_res = client1_res;
        client1_res += pair.second.CLIENT_STRING_ID;
        client2_res += pair.first.CLIENT_STRING_ID;
        client1_res += ",playerNum:";
        client1_res += pair.first.Player == PONE ? '1' : '2';
        client2_res += ",playerNum:";
        client2_res += pair.second.Player == PONE ? '1' : '2';

        std::wcout << "Client 1 res: " << convertString(client1_res) << std::endl;
        std::wcout << "Client 2 res: " << convertString(client2_res) << std::endl;

        pair.first.status = PLAYING;
        pair.second.status = PLAYING;

        std::thread* game_thread;
        game_thread->detach(); // Dont want main thread waiting for this ever
        initalize_client_lobby(game_thread, lobby_index, pair.first, pair.second);

        std::wcout << "Starting game thread" << std::endl;

        // start game thread
        game_thread = new std::thread(GAME_FOR_TWO_CLIENTS, lobby_index);
    } 
    
    // Send response to clients, if match was found then it will send a match found response
    for(auto index: activeIndexs){
        Client& curr = *clients[index]; // THIS WILL NOT BE NULLPTR
        if(curr.RES_TO_CLIENT_WAITING.length() != 0){
            std::wcout << "Curr client sending msg to: " << convertString(curr.CLIENT_STRING_ID) << ", msg: " << convertString(curr.RES_TO_CLIENT_WAITING) << std::endl;
            send(curr.Game.fd, curr.RES_TO_CLIENT_WAITING.c_str(), curr.RES_TO_CLIENT_WAITING.length(), 0);
        }
    }
}

void accept_poll_sockets(int fd){
    struct sockaddr_in* clientAddress;
    socklen_t client_addr_len = sizeof(*clientAddress);
    int clientSocketFD;

    while ((clientSocketFD = accept(fd, (struct sockaddr*)&clientAddress, &client_addr_len)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No pending connections available, continue the loop
            return;
        } else if (errno != EINTR) {
            std::wcout << "Poll socket, Accept failed: " << std::strerror(errno) << std::endl;
            return;
        }
    }


    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int byte_recieved = recv(clientSocketFD, (void*)buffer, sizeof(buffer), 0);
    if(byte_recieved <= 0){
        close(clientSocketFD);
        return; 
    }

    buffer[byte_recieved] = '\0';
    std::string client_bind_msg(buffer);

    if(client_bind_msg.find(CLIENT_ID_BIND_MSG) == std::string::npos){
        close(clientSocketFD);
        return; 
    }

    // Should have random string from client
    std::string client_rand_str = client_bind_msg.substr(BIND_INDEX_OF_START_RAND_STRING, client_bind_msg.length() - 1);

    // add client to server
    if(add_client(clientSocketFD, client_rand_str) < 0){
        close(clientSocketFD);
        return; 
    }

    std::wcout << "Client fd number accepted: " << clientSocketFD << std::endl;
    
    // Send success message
    send(clientSocketFD, (void*)SERVER_CLIENT_INIT_GOOD, sizeof(SERVER_CLIENT_INIT_GOOD), 0);
}

void signalHandler(int signal) {
    void *array[10];
    size_t size;

    // Get the array of void*'s for the backtrace.
    size = backtrace(array, 10);

    // Print out the backtrace to stderr.
    std::cerr << "Error: signal " << signal << ":" << std::endl;
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main(){

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);  // Catch segmentation faults
    std::signal(SIGABRT, signalHandler); // Handle abort signal
    std::signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE signals

    #ifdef SERVER_TERMINAL
const char* fifo_path = "/tmp/serverchessfifo";
    server_terminal_communication_fd = open(fifo_path, O_RDONLY | O_NONBLOCK);

    if (server_terminal_communication_fd == -1) {
        perror("open");
        
        char user_input;
        std::wcout << L"FIFO does not exist. Do you want to create it? (y/n): ";
        std::cin >> user_input;

        if (user_input == 'y' || user_input == 'Y') {
            if (mkfifo(fifo_path, 0777) == -1) {
                perror("mkfifo");
                return 1;
            } else {
                std::wcout << L"FIFO created successfully. Waiting until writer is available..." << std::endl;
                server_terminal_communication_fd = open(fifo_path, O_RDONLY | O_NONBLOCK);
                if (server_terminal_communication_fd == -1) {
                    perror("open");
                    return 1;
                }
            }
        } else {
            return 1;
        }
    }

    // Set the file descriptor for non-blocking mode
    int flags = fcntl(server_terminal_communication_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return 1;
    }
    if (fcntl(server_terminal_communication_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        return 1;
    }
    #endif

    //! Required
    init_moveset();

    // init server
    int poll_socket_fd = createTCPIPv4Socket();
    struct sockaddr_in* poll_socket_addr = createIPv4Address("", 2000);

    int client_game_sock_fd = createTCPIPv4Socket();
    struct sockaddr_in* client_game_sock_addr = createIPv4Address("", 2001);

    if (fcntl(poll_socket_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl failed");
        exit(EXIT_FAILURE);
    }

    if (fcntl(client_game_sock_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl failed");
        exit(EXIT_FAILURE);
    }

    if(bind(poll_socket_fd, (struct sockaddr*)poll_socket_addr, sizeof(*poll_socket_addr)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(listen(poll_socket_fd, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        close(poll_socket_fd);
        exit(EXIT_FAILURE);
    }

    if(bind(client_game_sock_fd, (struct sockaddr*)client_game_sock_addr, sizeof(*client_game_sock_addr)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        close(poll_socket_fd);
        exit(EXIT_FAILURE);
    }

    if(listen(client_game_sock_fd, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        close(poll_socket_fd);
        close(client_game_sock_fd);
        exit(EXIT_FAILURE);
    }

    // Server is setup now...

    std::wcout << "Server listening for clients..." << std::endl;

    //! BLOCKING ON ACCEPT ACCEPTS FIRST SOCKET AND ADDS BIND STR TO CLIENT INSTANCE - BLOCKING
    while(KILL_SERVER == false)
    {

        std::vector<Client*> activeClientsTracker;
        // Runs 1 time per 10 seconds
        send_poll_on_active_clients(activeClientsTracker);
        
        // Runs for 10 seconds
        for(int curr_second = 0; curr_second < 10; curr_second++){

            // Runs 4 times a second
            for(int curr_hundreth_milisecond = 0; curr_hundreth_milisecond < 4; curr_hundreth_milisecond++){
                // Accepts initial client poll sockets
                accept_poll_sockets(poll_socket_fd);     

                // Accepts initial client game sockets
                accept_game_socket(client_game_sock_fd); 

                // Checks for commands send by server terminal
                #ifdef SERVER_TERMINAL
                read_fifo();
                #endif

                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }

            // Runs 1 time a second
            dispatch_and_send_waiting_updates();
        }

        // Runs 1 time per 10 seconds
        recieve_poll_on_active_clients(activeClientsTracker);
    }

    return 0;
}


