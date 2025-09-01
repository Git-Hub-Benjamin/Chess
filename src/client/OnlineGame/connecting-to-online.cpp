#include "./online-mode.hpp"

// True  -- Match found
// False -- No match found
static bool checkMatchFoundInResponse(std::string& res, JOIN_GAME_INFO& obj){

    std::wcout << "In check_for_match_found, res: " << convertString(res) << std::endl;

    if(res.find(MATCH_FOUND) == std::string::npos)
        return false;

    std::wcout << "Match found, found" << std::endl;

    obj.opposingPlayerStr = res.substr(CLIENT_EXTRACT_OPPONENT_BIND_STR, CLIENT_EXTRACT_OPPONENT_BIND_STR_LEN);

    obj.myPlayerNum = static_cast<Player>(res[42] - 48);

    return true;
}

JOIN_GAME_INFO joinPrivateLobby(int fd){
    while(true){

        std::string oppBindStr;
        enum Owner playerNumForMatch;

        enter_private_lobby_code_menu();
        std::wstring lobby_code;
        std::wcin >> lobby_code;

        if(lobby_code.compare(L"!back") == 0)
            return JOIN_GAME_INFO(-1);
        
        if(lobby_code.length() != 5)
            continue;    
    
        // sendData to server to see if valid code
        std::string build_msg = CLIENT_LOBBY_JOIN_CODE + convertWString(lobby_code);
        sendData(fd, (char*)build_msg.c_str(), build_msg.length());

        // Recieve response from server
        char buffer[ONLINE_BUFFER_SIZE] = {0};
        int bytesRead = receiveData(fd, buffer, sizeof(buffer), 0);

        if (bytesRead == 0)
            return 0; // Server Error
        if (bytesRead < 0)
            return 1; // Client Error

        buffer[bytesRead] = '\0';

        std::string response(buffer);
        std::wcout << "Joining private lobby, from server: " << convertString(response) << std::endl;
        JOIN_GAME_INFO info;
        
        if(checkMatchFoundInResponse(response, info)){
            // Match found
            std::wcout << L"\033[2B\033[G" << std::flush;
            return info;
        } // Handle other response sent be server
    }
}

JOIN_GAME_INFO randomQueue(int fd){
    // Tell server to join the random queue
    if (sendData(fd, CLIENT_JOIN_RANDOM_QUEUE, sizeof(CLIENT_JOIN_RANDOM_QUEUE)) < 0)
        return JOIN_GAME_INFO(1);

    // Receive response from the server
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytesRead = receiveData(fd, buffer, sizeof(buffer), 0);
    
    if (bytesRead == 0)
        return 0; // Server Error
    if (bytesRead < 0)
        return 1; // Client Error

    // Format
    JOIN_GAME_INFO info;
    buffer[bytesRead] = '\0';
    std::string response(buffer);

    if (checkMatchFoundInResponse(response, info)) {
        std::wcout << L"\033[2B\033[G" << std::flush;
        return info;
    } else if (response != SERVER_CLIENT_JOIN_RAND_QUEUE_GOOD) {
        std::wcout << "Error joining random queue..." << std::endl;
        return JOIN_GAME_INFO(0);
    }

    int pipe_fd[2];
#ifdef __linux__
    if (pipe(pipe_fd) == -1)
        return JOIN_GAME_INFO(1);

    {
        bool conditionalStop = false;
        DisplayManager displayCodeAndWait(pipe_fd[0], conditionalStop);
        TerminalController terminalcontroller;
        display_rand_queue_menu(); // Display menu
        std::thread lobbyInput_t(&DisplayManager::watchMainThreadWhileInput, &displayCodeAndWait);

        while (true) {

            bytesRead = receiveData(fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response = buffer;

                if (checkMatchFoundInResponse(response, info)) {
                    write(pipe_fd[1], "1", 1);
                    if (lobbyInput_t.joinable())
                        lobbyInput_t.join();

                    std::wcout << L"\033[2B\033[G" << std::flush;
                    return info;
                } else {
                    std::wcout << L"\033[2B\033[G" << std::flush;
                    return JOIN_GAME_INFO(0);
                }
            } else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                return JOIN_GAME_INFO(1);
            }

            if (conditionalStop) { // If the input thread said to stop we will join with it
                if (lobbyInput_t.joinable())
                    lobbyInput_t.join();

                if (sendData(fd, (void*)CLIENT_LEAVE_RANDOM_QUEUE, sizeof(CLIENT_LEAVE_RANDOM_QUEUE), 0) < 0)
                    return JOIN_GAME_INFO(1);
                return JOIN_GAME_INFO(-1);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ensure some delay to prevent busy waiting
        }
    }
#elif _WIN32
    return JOIN_GAME_INFO(-1);
#endif
}

JOIN_GAME_INFO createPrivateLobby(int fd) {
    
    if (sendData(fd, CLIENT_CREATE_PRIVATE_LOBBY, sizeof(CLIENT_CREATE_PRIVATE_LOBBY)) < 0)
        return JOIN_GAME_INFO(1);

    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytesRead = receiveData(fd, buffer, sizeof(buffer), 0);

    buffer[bytesRead] = '\0';
    std::string response(buffer);

    if (bytesRead == 0)
        return 0; // Server Error
    if (bytesRead < 0)
        return 1; // Client Error

    int pipe_fd[2];
#ifdef __linux__
    if (pipe(pipe_fd) == -1) {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }
#elif _WIN32

#endif

    {
        bool conditionalStop = false;
        std::wstring lobbyCode = convertString(response.substr(CLIENT_INDEX_AFTER_COLON_IN_CREATE_LOBBY_CODE, bytesRead - 1));
        DisplayManager displayCodeAndWait(pipe_fd[0], conditionalStop);
        TerminalController terminalcontroller;
        displayPrivateLobbyCodeMenu(lobbyCode);
        std::thread lobbyInput_t(&DisplayManager::watchMainThreadWhileInput, &displayCodeAndWait);

        while (true) {

#ifdef __linux__
            bytesRead = receiveData(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
#elif _WIN32

#endif
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response = buffer;
                JOIN_GAME_INFO info;

                if (checkMatchFoundInResponse(response, info)) {
#ifdef __linux__
                    write(pipe_fd[1], "1", 1);
#elif _WIN32
#endif
                    if (lobbyInput_t.joinable())
                        lobbyInput_t.join();

                    std::wcout << L"\033[2B\033[G" << std::flush;
                    return info;
                } else {
                    std::wcout << L"\033[2B\033[G" << std::flush;
                    return JOIN_GAME_INFO(0);
                }
            } else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                return JOIN_GAME_INFO(1);
            }

            if (conditionalStop) { // If the input thread said to stop we will join with it
                if (lobbyInput_t.joinable())
                    lobbyInput_t.join();

                if (sendData(fd, CLIENT_LEAVE_RANDOM_QUEUE, sizeof(CLIENT_LEAVE_RANDOM_QUEUE)) < 0)
                    return JOIN_GAME_INFO(1);
                return JOIN_GAME_INFO(-1);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ensure some delay to prevent busy waiting
        }
    }
}

/*
JOIN_GAME_INFO createPrivateLobby2(int fd) {
    
goto skip;
    if (sendData(fd, (void*)CLIENT_CREATE_PRIVATE_LOBBY, sizeof(CLIENT_CREATE_PRIVATE_LOBBY), 0) < 0)
        return JOIN_GAME_INFO(1);
skip:
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytesRead = -1; //receiveData(fd, (void*)buffer, sizeof(buffer), 0);

    buffer[bytesRead] = '\0';
    std::string response(buffer);

    if (bytesRead == 0)
        return 0; // Server Error
    if (bytesRead < 0)
        return 1; // Client Error

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }

    {
        bool conditionalStop = false;
        std::wstring lobbyCode = L"12345"; //convertString(response.substr(CLIENT_INDEX_AFTER_COLON_IN_CREATE_LOBBY_CODE, bytesRead - 1));
        DisplayManager displayCodeAndWait(pipe_fd[0], conditionalStop);
        TerminalController terminalcontroller;
        displayPrivateLobbyCodeMenu(lobbyCode);
        std::thread lobbyInput_t(&DisplayManager::watchMainThreadWhileInput, &displayCodeAndWait);
        int count = 0;
        while (true) {
            if (count == 30) {
                write(pipe_fd[1], "1", 1);

                if (lobbyInput_t.joinable())
                    lobbyInput_t.join();

                std::wcout << L"\033[2B\033[G" << std::flush;
                return JOIN_GAME_INFO(PlayerOne, std::string("12345")); //info;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ensure some delay to prevent busy waiting
            count++;
        }
    }
}

*/