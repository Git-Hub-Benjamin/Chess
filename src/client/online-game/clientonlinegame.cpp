#include "./online-mode.hpp"
 
std::string ONLINE_PLAYER_ID;
static std::atomic_bool ONLINE_QUIT(false);

void poll_to_server(int poll_fd) {
    while (!ONLINE_QUIT) {
        char buffer[sizeof(SERVER_POLL_MSG)] = {0};

        int bytes_read = recv(poll_fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);

        if (bytes_read == 0) {
            break; // Server closed connection
        } else if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string msg(buffer);
            std::wcout << "From server: " << convertString(msg) << std::endl;
            send(poll_fd, (void*)CLIENT_POLL_MSG, sizeof(CLIENT_POLL_MSG), 0);
        } else if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "Error occurred in socket" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::this_thread::sleep_for(std::chrono::seconds(5)); // Poll every 5 seconds
    }
}


enum CONNECT_TO_SERVER {
    CONNECTING,
    ERROR,
    CONNECTED
};

static std::atomic<CONNECT_TO_SERVER> status(CONNECTING);

void loading_connecting_to_server(){
    
    std::wstring dots = L".";
    int count = 0;

    while (count < 3) {
        
        if (status == ERROR || (status == CONNECTED && count > 3)) {
            break;
        }

        std::wcout << L"\rConnecting to server" << dots << std::flush;
        count++;
        dots += L".";
        std::this_thread::sleep_for(std::chrono::milliseconds(750));
    }

    std::wcout << std::endl;
}


void error_connecting(std::wstring&& msg, std::thread& animation_t) {
    set_terminal_color(RED);
    std::wcout << msg << std::endl;
    set_terminal_color(DEFAULT);
    status = ERROR;
    if (animation_t.joinable())
        animation_t.join();
}

// 0 no errors
// 1 errors
int connect_to_server(int poll_fd, struct sockaddr_in* poll_addr, int game_fd, struct sockaddr_in* game_addr){
    
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    std::thread init_connection_animation_t(loading_connecting_to_server);

    if (connect(poll_fd, (const sockaddr*)poll_addr, sizeof(*poll_addr)) < 0) {
        error_connecting(std::wstring(L"Connection to server_poll socket failed - Server is probably down..."), init_connection_animation_t); 
        return 1;
    }


    // Send bind string
    int bytesRead = recv(poll_fd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        error_connecting(std::wstring(L"Failed to receive from server_poll socket..."), init_connection_animation_t); 
        return 1;
    }

    std::string bind_str(buffer);
    ONLINE_PLAYER_ID = bind_str.substr(BIND_INDEX_OF_START_RAND_STRING, bytesRead - 1);;

    if(connect(game_fd, (const sockaddr*)game_addr, sizeof(*game_addr)) < 0) {
        error_connecting(std::wstring(L"Connection to game_server socket failed..."), init_connection_animation_t); 
        return 1;
    }

    // Send bind string
    send(game_fd, bind_str.c_str(), bind_str.length(), 0);

    // Receive msg from server
    if(recv(game_fd, buffer, sizeof(buffer) - 1, 0) <= 0){
        error_connecting(std::wstring(L"Failed to receive from game_server socket..."), init_connection_animation_t); 
        return 1;
    }

    status = CONNECTED;

    // Make sure the animation thread joins before continuing execution
    if(init_connection_animation_t.joinable())
        init_connection_animation_t.join();

    return 0;

}

int online_game() {

    int client_game_socket_fd = createTCPIPv4Socket();
    int client_poll_socket_fd = createTCPIPv4Socket();
    struct sockaddr_in* server_poll_addr = createIPv4Address("127.0.0.1", 2000);
    struct sockaddr_in* server_game_addr = createIPv4Address("127.0.0.1", 2001);

    int res = connect_to_server(client_poll_socket_fd, server_poll_addr, client_game_socket_fd, server_game_addr);

    if(res == 1)
        return 1;

    // Start polling to the server to keep the connection alive
    std::thread poll_thread(poll_to_server, client_poll_socket_fd);

    // Server was able to bind our sockets together, we are now waiting but without a JOIN_MODE
    // On the client we will now show the game modes (Private lobby or random queue)
    // If we choose one we will tell the server

    while(true){
        online_menu(convertString(ONLINE_PLAYER_ID));
        int online_opt = get_menu_option();
        JOIN_GAME_INFO info;
        if(online_opt == 1){
            // Random queue
            info = randomQueue(client_game_socket_fd);
        }else if(online_opt == 2){
            // Create private lobby
            info = createPrivateLobby(client_game_socket_fd);
        }else if(online_opt == 3){
            // Join private lobby
            info = joinPrivateLobby(client_game_socket_fd);
        }else{
            // Tell server to close Game Socket, this will also clean up the poll socket associated with it
            ONLINE_QUIT = true;
            // Wait for poll thread to join main thread
            if(poll_thread.joinable())
                poll_thread.join();
            close(client_game_socket_fd);
            close(client_poll_socket_fd);
            return 0;
        }

        if (info.joinState == 2) {
            std::wcout << "My player num sent from server: " << (info.myPlayerNum == PlayerOne ? L"Player One" : L"Player Two") << std::endl;
            StandardOnlineChessGame Game(client_game_socket_fd, info.myPlayerNum, convertString(info.opposingPlayerStr));
            Game.startGame();
        } else if (info.joinState == 1) 
            std::wcout << "Something went wrong on your end..." << std::endl;
        else if (info.joinState == 0)
            std::wcout << "Something went wrong on theo server end..." << std::endl;
    }   
}
