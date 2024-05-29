#include "./online-mode.hpp"

// True  -- Match found
// False -- No match found
static bool check_for_match_found(std::string& res, std::string& out_bin_str, enum Owner& outSelfPlayerNum){

    std::wcout << "In check_for_match_found, res: " << convertString(res) << std::endl;

    if(res.find(MATCH_FOUND) == std::string::npos)
        return false;

    std::wcout << "Match found, found" << std::endl;

    out_bin_str = res.substr(CLIENT_EXTRACT_OPPONENT_BIND_STR, CLIENT_EXTRACT_OPPONENT_BIND_STR_LEN);

    outSelfPlayerNum = static_cast<enum Owner>(res[42] - 48);

    return true;
}

int join_private_lobby(int fd){
    while(true){

        std::string oppBindStr;
        enum Owner playerNumForMatch;

        enter_private_lobby_code_menu();
        std::wstring lobby_code;
        std::wcin >> lobby_code;

        if(lobby_code.compare(L"!back") == 0)
            break;
        
        if(lobby_code.length() != 5)
            continue;    
    
        // Send to server to see if valid code
        std::string build_msg = CLIENT_LOBBY_JOIN_CODE + convertWString(lobby_code);
        send(fd, (void*)build_msg.c_str(), build_msg.length(), 0);

        // Recieve response from server
        char buffer[ONLINE_BUFFER_SIZE] = {0};
        int bytes_recieved = recv(fd, (void*)buffer, sizeof(buffer), 0);
        buffer[bytes_recieved] = '\0';
        std::string res_from_server(buffer);
        if(check_for_match_found(res_from_server, oppBindStr, playerNumForMatch)){
            // Match found
            std::wcout << L"\033[2B\033[G" << std::flush;
            game_loop(fd, playerNumForMatch, oppBindStr);
            return 0;
        } // Handle other response sent be server
    }
    return 0;
}

int rand_queue_wait(int fd){
    // Tell server to join the random queue
    send(fd, (void*)CLIENT_JOIN_RANDOM_QUEUE, sizeof(CLIENT_JOIN_RANDOM_QUEUE), 0);

    // Receive response from the server
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), 0);
    
    // Format
    std::string oppBindStr;
    enum Owner playerNumForMatch;
    buffer[bytes_read] = '\0';
    std::string res_from_server(buffer);

    std::wcout << "From server: " << convertString(res_from_server) << std::endl;

    // Chance that when joining the random queue you could be queued immediently, this happened to me
    if(res_from_server.compare(SERVER_CLIENT_JOIN_RAND_QUEUE_GOOD) != 0)
    {
        std::wcout << "Error joining random queue..." << std::endl;
        return 1;
    }else if(res_from_server.compare(MATCH_FOUND) == 0){
        std::wcout << L"\033[2B\033[G" << std::flush;
        game_loop(fd, playerNumForMatch, oppBindStr);
        return 0;
    }

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }

    {
        DisplayManager displayCodeAndWait(pipe_fd[0]);
        TerminalController terminalcontroller;
        std::atomic_bool& stop_from_input_t = displayCodeAndWait.stop_display;
        display_rand_queue_menu();
        std::thread lobbyInput_t(&DisplayManager::start_input, &displayCodeAndWait);

        while (true) {

            std::string oppBindStr;
            enum Owner playerNumForMatch;
            char buffer[ONLINE_BUFFER_SIZE] = {0};

            int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::string res_from_server(buffer);

                if (check_for_match_found(res_from_server, oppBindStr, playerNumForMatch)) {
                    write(pipe_fd[1], "1", 1);
                    if (lobbyInput_t.joinable())
                        lobbyInput_t.join();

                    std::wcout << L"\033[2B\033[G" << std::flush;
                    game_loop(fd, playerNumForMatch, oppBindStr);
                    return 0;
                } else {
                    std::wcout << L"\033[2B\033[G" << std::flush;
                    std::wcout << "Error joining lobby for some reason..." << std::endl;
                    return 1;
                }
            } else if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::wcout << "Error occurred in socket" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (stop_from_input_t) {
                if (lobbyInput_t.joinable())
                    lobbyInput_t.join();

                std::wcout << "Stop from input thread was set to true" << std::endl;

                send(fd, (void*)CLIENT_LEAVE_RANDOM_QUEUE, sizeof(CLIENT_LEAVE_RANDOM_QUEUE), 0);
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ensure some delay to prevent busy waiting
        }
    }
}

int create_private_lobby(int fd) {
    send(fd, (void*)CLIENT_CREATE_PRIVATE_LOBBY, sizeof(CLIENT_CREATE_PRIVATE_LOBBY), 0);

    std::wcout << "Lobby code requested from server..." << std::endl;

    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_recieved = recv(fd, (void*)buffer, sizeof(buffer), 0);

    buffer[bytes_recieved] = '\0';
    std::string res_from_server(buffer);

    if (bytes_recieved <= 0 || res_from_server.find(SERVER_CREATE_LOBBY_GOOD) == std::string::npos) {
        std::wcout << "Server error creating a private lobby, try again later..." << std::endl;
        return 1;
    }

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }

    std::wstring lobby_code = convertString(res_from_server.substr(CLIENT_INDEX_AFTER_COLON_IN_CREATE_LOBBY_CODE, bytes_recieved - 1));

    {
        DisplayManager displayCodeAndWait(pipe_fd[0]);
        TerminalController terminalcontroller;
        std::atomic_bool& stop_from_input_t = displayCodeAndWait.stop_display;
        display_private_lobby_code_menu(lobby_code);
        std::thread lobbyInput_t(&DisplayManager::start_input, &displayCodeAndWait);

        while (true) {

            std::string oppBindStr;
            enum Owner playerNumForMatch;
            char buffer[ONLINE_BUFFER_SIZE] = {0};

            int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::string res_from_server(buffer);

                if (check_for_match_found(res_from_server, oppBindStr, playerNumForMatch)) {
                    write(pipe_fd[1], "1", 1);
                    if (lobbyInput_t.joinable())
                        lobbyInput_t.join();

                    std::wcout << L"\033[2B\033[G" << std::flush;
                    game_loop(fd, playerNumForMatch, oppBindStr);
                    return 0;
                } else {
                    std::wcout << L"\033[2B\033[G" << std::flush;
                    std::wcout << "Error creating/joining lobby for some reason..." << std::endl;
                    send(fd, (void*)CLIENT_CLOSE_LOBBY, sizeof(CLIENT_CLOSE_LOBBY), 0);
                    return 1;
                }
            } else if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::wcout << "Error occurred in socket" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (stop_from_input_t) {
                if (lobbyInput_t.joinable())
                    lobbyInput_t.join();

                send(fd, (void*)CLIENT_CLOSE_LOBBY, sizeof(CLIENT_CLOSE_LOBBY), 0);
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ensure some delay to prevent busy waiting
        }
    }
}