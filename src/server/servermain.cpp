#include "../chess.hpp"
#include "./socket/socketutil.h"
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


#define MAX_CLIENT 8

std::mutex mtx;
static Client* clients[MAX_CLIENT] = { nullptr };
std::unordered_map<std::string, Client*> PRIVATE_LOBBY_CODES;

void handleOption(){
    // this should be a client only function so we need to refactor
}

std::string charP_to_string(const char* str){
    std::string temp = str;
    return std::string(temp.begin(), temp.end());
}

std::wstring string_to_wstring(std::string &str){
    return std::wstring(str.begin(), str.end());
}

void client_access_lock() { mtx.lock(); }
void client_access_unlock() { mtx.unlock(); }


void close_client(int index){
    client_access_lock();

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
    
    client_access_unlock();
}

int add_client(int client_fd, std::string& bind_msg){
    client_access_lock();
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
    client_access_unlock();
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


//! only close clients from here
void poll_for_active_clients(int fd){
    while(true){
        std::vector<Client*> check_active;

        client_access_lock();
        for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
            // Make sure client is in slot and bound
            if(clients[client_index] != nullptr && clients[client_index]->status != UNBOUND)
                check_active.push_back(clients[client_index]);
        }
        client_access_unlock();

        for(Client* client: check_active){
            // Maybe make this non blocking
            std::wcout << "Client fd: " << client->Poll.fd << ", Bind: " << convertString(client->CLIENT_STRING_ID) << std::endl;
            send(client->Poll.fd, (void*)SERVER_POLL_MSG, sizeof(SERVER_POLL_MSG), 0); 
        }

        std::wcout << "Sending poll msg to (" << (waiting_players_count() + playing_players_count()) << ") client(s)" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(10));

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

void accept_client_game_socket(int game_sock_fd){
    while(true){

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        struct sockaddr_in* clientAddress;
        socklen_t client_addr_len = sizeof(*clientAddress);

        int res = accept(game_sock_fd, (struct sockaddr*)&clientAddress, (socklen_t*)&client_addr_len);
        if(res < 0){
            std::wcout << "Game Socket, Accept failed." << std::endl;
            continue;
        }
        char buffer[ONLINE_BUFFER_SIZE] = {0};
        int clientSocketFD = res;
        int byte_recieved = recv(clientSocketFD, (void*)buffer, sizeof(buffer), 0);
        if(byte_recieved <= 0){
            // Not sure when this would happen
            send(clientSocketFD, (void*)SERVER_CLIENT_INIT_FAIL, sizeof(SERVER_CLIENT_INIT_FAIL), 0);
            continue;
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
            continue;
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



std::unordered_map<std::string, std::string>& process_client_msg(std::string& msg) {
    // assume bytes were sent
    std::unordered_map<std::string, std::string>* client_msg = new std::unordered_map<std::string, std::string>;

    int left = 0;
    int right = 0;
    
    while (true) {
        while (right != msg.length() && msg[right] != '\n' && msg[right] != ':')
            right++;

        if (msg.length() == right || msg[right] == '\n') {
            std::cerr << "Messages should always be tailed with a new line, but always have a value with a colon" << std::endl;
            exit(EXIT_FAILURE);
        }

        // msg[right] should be a colon

        if (msg.length() == right + 1 || 0 == right - 1) {
            std::cerr << "Message from client is not correct should be <key>:<value>NEWLINE" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string key(msg.substr(left, right - left));  // Fix the substring indices

        right++;
        left = right;

        while (right != msg.length() && msg[right] != '\n') {
            right++;
        }

        (*client_msg)[key] = msg.substr(left, right - left);  // Fix the substring indices
        
        if (right == msg.length()) {
            // means that we reached the end of the message
            break;
        }

        right++;  // Skip the newline character
        left = right;
    }
    return *client_msg;
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

void GAME_FOR_TWO_CLIENTS(Client& client1, Client& client2){
    // GIVE TIME FOR EACH CLIENT TO JOIN BEFORE STARTING GAME
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if(both_clients_joined_successfully(client1, client2)){
        // Start game
    }
}

void schedule_rand_queue_clients(std::vector<std::pair<int, int>>& vec){
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
            vec.push_back({ready_client_one_index, ready_client_two_index});
            ready_clients = 0;
            ready_client_one_index = -1;
            ready_client_two_index = -1;
        }
    }
}

//* IN CLIENT ACCESS LOCK
// assume not nullptr and the curr_client->status is waiting
std::string client_update_game_mode(Client* curr_client, std::vector<std::pair<int, int>>& vec) {
    std::string build_string = "";
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    // for every waiting client we are going to send the current active players
    // it is up to the client if they want to display / use the information

    // server activity to clients
    int byte_recieved = recv(curr_client->Game.fd, buffer, sizeof(buffer), MSG_DONTWAIT);
    
    // Means that the client did not choose a gamemode yet
    if (byte_recieved < 0)
        return build_string;

    // New update on the client side, maybe they changed to random queue
    buffer[byte_recieved] = '\0';
    std::string msg_from_client(buffer);

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
            // Schedule two clients
            // We dont have to do build_string += MATCH_FOUND because the dispatcher will do that 
            vec.push_back({maker_of_lobby_client->Game.index, curr_client->Game.index});
        }
    }

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
void client_waiting_to_playing(int client_index){
    Client& curr = *clients[client_index];
    curr.status = PLAYING;
    curr.RES_TO_CLIENT_WAITING = MATCH_FOUND;
}

// Runs random queue dispatcher 1 time a second, and sends updates to 
// waiting clients 1 time every 3 seconds
void dispatch_and_send_waiting_updates(){
    
    while(true){

        client_access_lock();

        std::string server_activity_msg = get_waiting_updates();
        std::vector<std::pair<int, int>> CLIENTS_TO_DISPATCH;
        std::vector<int> activeIndexs;

        for(int client_index = 0; client_index < MAX_CLIENT; client_index++){
            
            Client* curr_client = clients[client_index];
            
            if(curr_client == nullptr || curr_client->status != WAITING)
                continue;

            std::wcout << "Waiting client game-fd: " << curr_client->Game.fd << std::endl;

            activeIndexs.push_back(client_index);

            std::string res_to_client = server_activity_msg;

            res_to_client += client_update_game_mode(curr_client, CLIENTS_TO_DISPATCH);

            curr_client->RES_TO_CLIENT_WAITING = res_to_client;
        }

        schedule_rand_queue_clients(CLIENTS_TO_DISPATCH); 

        // Schedule matches found
        for(auto pair: CLIENTS_TO_DISPATCH){
            client_waiting_to_playing(pair.first); // This will set matchfound for the client
            client_waiting_to_playing(pair.second); // This too, then it will send it below regulary
            std::thread(GAME_FOR_TWO_CLIENTS, std::ref(*clients[pair.first]), std::ref(*clients[pair.second]));
        } 
        
        // Send response to clients, if match was found then it will send a match found response
        for(auto index: activeIndexs){
            Client& curr = *clients[index]; // THIS WILL NOT BE NULLPTR
            send(curr.Game.fd, curr.RES_TO_CLIENT_WAITING.c_str(), curr.RES_TO_CLIENT_WAITING.length(), 0);
        }

        client_access_unlock();
        
        // Repeat every second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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

    // init server
    int poll_socket_fd = createTCPIPv4Socket();
    struct sockaddr_in* poll_socket_addr = createIPv4Address("", 2000);

    int client_game_sock_fd = createTCPIPv4Socket();
    struct sockaddr_in* client_game_sock_addr = createIPv4Address("", 2001);

    // if (fcntl(poll_socket_fd, F_SETFL, O_NONBLOCK) < 0) {
    //     perror("fcntl failed");
    //     exit(EXIT_FAILURE);
    // }

    // if (fcntl(client_game_sock_fd, F_SETFL, O_NONBLOCK) < 0) {
    //     perror("fcntl failed");
    //     exit(EXIT_FAILURE);
    // }

    if(bind(poll_socket_fd, (struct sockaddr*)poll_socket_addr, sizeof(*poll_socket_addr)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(listen(poll_socket_fd, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(bind(client_game_sock_fd, (struct sockaddr*)client_game_sock_addr, sizeof(*client_game_sock_addr)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(listen(client_game_sock_fd, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        exit(EXIT_FAILURE);
    }

    //! STOPS FOR 10 SECOND - REMOVING INACTIVE CLIENTS - BLOCKING
    // thread for poll
    std::thread poll_thread(poll_for_active_clients, client_game_sock_fd);

    //! BLOCKING ON ACCEPT - ACCEPTS SECOND SOCKET & BINDS TO FIRST SOCKET - BLOCKING
    // thread for accpeting poll socket
    std::thread accept_poll_socket(accept_client_game_socket, client_game_sock_fd);

    //** NON BLOCKING - SENDS CONTINOUS UPDATES TO CLIENT ABOUT QUEUING FOR GAME 
    std::thread monitor_waiting_clients_t(dispatch_and_send_waiting_updates);

    // Server is setup now...

    std::wcout << "Server listening for clients..." << std::endl;

    //! BLOCKING ON ACCEPT ACCEPTS FIRST SOCKET AND ADDS BIND STR TO CLIENT INSTANCE - BLOCKING
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        struct sockaddr_in* clientAddress;
        socklen_t client_addr_len = sizeof(*clientAddress);
        int clientSocketFD;

        if((clientSocketFD = accept(poll_socket_fd, (struct sockaddr*)&clientAddress, &client_addr_len)) < 0){
            std::wcout << "Poll socket, Accept failed." << std::endl;
            continue;
        }

        char buffer[ONLINE_BUFFER_SIZE] = {0};
        int byte_recieved = recv(clientSocketFD, (void*)buffer, sizeof(buffer), 0);
        if(byte_recieved <= 0){
            close(clientSocketFD);
            continue; 
        }

        buffer[byte_recieved] = '\0';
        std::string client_bind_msg(buffer);

        if(client_bind_msg.find(CLIENT_ID_BIND_MSG) == std::string::npos){
            close(clientSocketFD);
            continue; 
        }

        // Should have random string from client
        std::string client_rand_str = client_bind_msg.substr(BIND_INDEX_OF_START_RAND_STRING, client_bind_msg.length() - 1);

        // add client to server
        if(add_client(clientSocketFD, client_rand_str) < 0){
            close(clientSocketFD);
            continue; 
        }

        std::wcout << "Client fd number accepted: " << clientSocketFD << std::endl;
        
        // Send success message
        send(clientSocketFD, (void*)SERVER_CLIENT_INIT_GOOD, sizeof(SERVER_CLIENT_INIT_GOOD), 0);
    }

    return 0;
}
