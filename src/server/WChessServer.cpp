#include "server.hpp"
#include "./server-terminal-communication/servercommands.hpp"

//! Send the poll message at 0 seconds, wait 10 seconds then run a different function
//! that recieves the poll message and does the same thing as poll_for_active_clients but split it in 2
void WChessServer::sendPollMsg(std::vector<Client*>& activeClients){

    m_ClientPlayingCount = 0;
    m_ClientOnlineCount = 0;
    m_ClientWaitingCount = 0;

    for (auto& [str, client]: m_Clients) {
        activeClients.push_back(&client);
        
        //! Maybe make this non blocking, and check for errors
        send(client.pollFd, (void*)SERVER_POLL_MSG, sizeof(SERVER_POLL_MSG), 0); 

        if (client.status == PLAYING)
            m_ClientPlayingCount++;
        else if(client.status == WAITING)
            m_ClientWaitingCount++;
        else
            m_ClientOnlineCount++;
    }

    m_ClientOnlineCount += m_ClientPlayingCount + m_ClientWaitingCount;

}

void WChessServer::addClient(int fd, std::string& bind){
    std::wcout << "Adding a client, key: " << convertString(bind) << ", Fd: " << fd << std::endl;
    m_Clients[bind] = Client(fd, bind);
    std::wcout << "Clients string id: " << convertString(m_Clients[bind].CLIENT_STRING_ID) << std::endl;
}

void WChessServer::acceptClientPollSocket(){

    // Client info
    struct sockaddr_in clientAddress; 
    socklen_t client_addr_len = sizeof(clientAddress); 
    int clientSocketFD;

    while ((clientSocketFD = accept(m_ServerPollFd, (struct sockaddr*)&clientAddress, &client_addr_len)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No pending connections available, continue the loop
            return;
        } else if (errno != EINTR) {
            std::wcout << "Poll socket, Accept failed: " << std::strerror(errno) << std::endl;
            return;
        }
    }

    std::string str = CLIENT_ID_BIND_MSG;
    std::string bind = generateRandomString(CLIENT_ID_RAND_STRING_LEN);
    std::string bindStr = str + bind;

    // add client to server
    addClient(clientSocketFD, bind);

    //! Handle fail send
    send(clientSocketFD, bindStr.c_str(), bindStr.length(), 0);
}

bool WChessServer::clientBindSockets(std::string bind){
    return m_Clients.find(bind) != m_Clients.end();
}

void WChessServer::acceptClientGameSocket(){

    // Client info
    struct sockaddr_in clientAddress; 
    socklen_t client_addr_len = sizeof(clientAddress); 

    int clientSocketFD;

    while ((clientSocketFD = accept(m_ServerGameFd, (struct sockaddr*)&clientAddress, (socklen_t*)&client_addr_len)) < 0) {
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
    std::string client_bind_id = std::string(buffer).substr(BIND_INDEX_OF_START_RAND_STRING, byte_recieved - 1);

    std::wcout << "Client sent this string to bind: " << convertString(client_bind_id) << std::endl;
    if (clientBindSockets(client_bind_id)) {
        std::lock_guard<std::mutex> lock(clientAccess);
        m_Clients[client_bind_id].gameFd = clientSocketFD;
        m_Clients[client_bind_id].status = WAITING;
    } else {
        std::wcout << "Could not find a matching client with this random string" << std::endl;
        send(clientSocketFD, (void*)SERVER_CLIENT_INIT_FAIL, sizeof(SERVER_CLIENT_INIT_FAIL), 0);
        return;
    }

    std::wcout << "Successfully bound : " << convertString(client_bind_id) << ", Poll: " << m_Clients[client_bind_id].pollFd << ", Game: " << m_Clients[client_bind_id].gameFd << std::endl;

    // Send success response to client
    send(clientSocketFD, (void*)SERVER_CLIENT_INIT_GOOD, sizeof(SERVER_CLIENT_INIT_GOOD), 0);
}

//! only close clients from here
void WChessServer::recievePollMsg(std::vector<Client*>& check_active){
    std::lock_guard<std::mutex> lock(clientAccess);
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    for(Client* client: check_active){
        std::wcout << "Reading from this this fd: " << client->pollFd << std::endl;
        int bytes_recieved = recv(client->pollFd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
        buffer[bytes_recieved] = '\0';

        std::string msg_from_client(buffer);

        std::wcout << "Msg from client: " << convertString(msg_from_client) << ", Bytes recieved: " << bytes_recieved << std::endl;

        if(bytes_recieved <= 0 || std::string(buffer).compare(CLIENT_POLL_MSG) != 0) {
            closeClientPrivateRooms(client->CLIENT_STRING_ID);
            m_Clients.erase(client->CLIENT_STRING_ID);
        }
    }

    for (auto it = m_Clients.begin(); it != m_Clients.end(); ) {  // Note the use of `it` here
        if (it->second.status == DISCONNECTED_KILL || 
           (it->second.status == UNBOUND && 3 < std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - it->second.timeSinceUnbound).count())) {
            closeClientPrivateRooms(it->first);
            it = m_Clients.erase(it);  
        } else 
            ++it;  // Only increment the iterator if no erase happened
    }
}

Owner WChessServer::fiftyFiftyTurnGenerator() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, 1);
    if(distribution(gen) + 1 == 1){ // Client 1 will be PONE, meaning will go first
        return PONE;
    }
    return PTWO;
}

void WChessServer::newOnlineGame(Client& client1, Client& client2){
    enum Owner playerSelect = fiftyFiftyTurnGenerator();

    client1.Player = playerSelect;
    client2.Player = (playerSelect == PONE ? PTWO : PONE);;

    std::string client1_res = MATCH_FOUND;
    client1_res += ",opponent:";
    std::string client2_res = client1_res;
    client1_res += client2.CLIENT_STRING_ID;
    client2_res += client1.CLIENT_STRING_ID;
    client1_res += ",playerNum:";
    client1_res += client1.Player == PONE ? '1' : '2';
    client2_res += ",playerNum:";
    client2_res += client2.Player == PONE ? '1' : '2';

    std::wcout << "Client 1 res: " << convertString(client1_res) << std::endl;
    std::wcout << "Client 2 res: " << convertString(client2_res) << std::endl;

    client1.status = PLAYING;
    client2.status = PLAYING;

    client1.RES_TO_CLIENT_WAITING = client1_res;
    client2.RES_TO_CLIENT_WAITING = client2_res;

    GAME_MODE OnlineGameMode = STANDARD_CHESSGAME;
    std::shared_ptr<OnlineChessGame> Game;

    // Create new lobby, this is also where we would change which chess game we would be creating
    // Since the constructor expects the first client passed to be player one we need to check for that
    if(client1.Player == PONE)
        Game = std::make_shared<StandardServerOnlineChessGame>(OnlineGameMode, client1, client2);
    else
        Game = std::make_shared<StandardServerOnlineChessGame>(OnlineGameMode, client2, client2);
    
    // Create thread to execute in lobbies init function, it will wait to main says it is good to go
    switch (OnlineGameMode) {
        case STANDARD_CHESSGAME:
            std::thread game_t(&StandardServerOnlineChessGame::startGame, static_cast<StandardServerOnlineChessGame*>(Game.get()));
            game_t.detach();
            static_cast<StandardServerOnlineChessGame*>(Game.get())->setStart();
        break;
    }
}

void WChessServer::randomQueue(){
    
    std::string readyClient;

    for (auto [str, client]: m_Clients) {
        if (client.status == WAITING && client.clientJoinMode == QUEUE_RANDOM) {
            if (readyClient.length() == 0)
                readyClient = str;
            else {
                // Start game for two clients
                newOnlineGame(m_Clients[readyClient], client);
            }
        }
    }
}

std::string WChessServer::createPrivateRoom(std::string str){
    closeClientPrivateRooms(str);
    std::string rand_code = generateRandomString(5);
    m_LobbyCodes[str] = rand_code;
    return rand_code;
}

void WChessServer::closeClientPrivateRooms(std::string clientStr) {
    for (auto it = m_LobbyCodes.begin(); it != m_LobbyCodes.end(); ) {
        if (it->second == clientStr) {
            it = m_LobbyCodes.erase(it);  
        } else {
            ++it;  // Move to the next element
        }
    }
}


bool WChessServer::joinClientPrivateRoom(std::string code) {
    return m_LobbyCodes.find(code) != m_LobbyCodes.end(); 
}

void WChessServer::waitingClientUpdate() {
    for (auto [str, client]: m_Clients) {

        std::string build_string;

        char buffer[ONLINE_BUFFER_SIZE] = {0};

        // for every waiting client we are going to send the current active players
        // it is up to the client if they want to display / use the information

        // server activity to clients
        int byte_recieved = recv(client.gameFd, buffer, sizeof(buffer), MSG_DONTWAIT);
        
        // Means that the client did not choose a gamemode yet, or client connection closed
        if (byte_recieved <= 0)
            continue;

        // New update on the client side, maybe they changed to random queue
        buffer[byte_recieved] = '\0';
        std::string msg_from_client(buffer);

        std::wcout << "=====\nMsg from " << convertString(client.CLIENT_STRING_ID) << ": " << convertString(msg_from_client) << std::endl; 

        // The client could be doing the following...

        // 1. Closing their Game Socket FD, We need to clean up the Client instance and the poll socket associated with it
        // 2. Joining the random queue
        // 3. Leaving the random queue
        // 4. Creating a private lobby
        // 5. Closing a private lobby
        // 6. Joining a private lobby

        if(byte_recieved == 0)
            client.status = DISCONNECTED_KILL;
        else if(msg_from_client.compare(CLIENT_JOIN_RANDOM_QUEUE) == 0){
            // Handling 2. joining random queue
            build_string += SERVER_CLIENT_JOIN_RAND_QUEUE_GOOD;
            client.clientJoinMode = QUEUE_RANDOM;
        }else if(msg_from_client.compare(CLIENT_LEAVE_RANDOM_QUEUE) == 0){
            // Handling 3. leaving random queue
            build_string += SERVER_CLIENT_LEAVE_RAND_QUEUE_GOOD;
            client.clientJoinMode = NONE_SELECTED;
        }else if(msg_from_client.compare(CLIENT_CREATE_PRIVATE_LOBBY) == 0){
            // Handling 4. creating a private lobby
            build_string += SERVER_CREATE_LOBBY_GOOD;
            build_string += createPrivateRoom(str);
        }else if(msg_from_client.compare(CLIENT_CLOSE_LOBBY) == 0){
            // Handling 5. closing a private lobby
            build_string += SERVER_CLIENT_CLOSE_LOBBY_GOOD;
            closeClientPrivateRooms(str);
        }else{
            // Handling 6. joining private lobby
            std::string codeToJoin = msg_from_client.substr(INDEX_AFTER_COLON_JOIN_CODE, byte_recieved - 1);
            
            if (joinClientPrivateRoom(codeToJoin)) {
                newOnlineGame(m_Clients[m_LobbyCodes[codeToJoin]], client);
            } else
                build_string += SERVER_CLIENT_JOIN_PRIVATE_LOBBY_BAD; // invalid code, or room is closed now
        }

        if (build_string.length() != 0)
            send(client.gameFd, build_string.c_str(), build_string.length(), 0);

    }
}

// Runs random queue dispatcher 1 time a second, and sends updates to 
void WChessServer::dispatchClients(){
    
    std::lock_guard<std::mutex> lock(clientAccess);

    waitingClientUpdate();

    randomQueue();
    
    // Send response to clients, if match was found then it will send a match found response
    for (auto [str, client]: m_Clients) {
        if (client.RES_TO_CLIENT_WAITING.length() != 0)
            send(client.gameFd, client.RES_TO_CLIENT_WAITING.c_str(), client.RES_TO_CLIENT_WAITING.length(), 0);
    }
}


void WChessServer::fifo_displayClientInfo(Client& curr){
    std::wcout << "\n= = = = = = =" << std::endl;
    std::wcout << "Client string: " << convertString(curr.CLIENT_STRING_ID) << ", \nFile descriptor (Game/Poll): " << curr.gameFd << "/" << curr.pollFd << ", \nStatus: " << (curr.status == UNBOUND ? L"Unbound" : curr.status == WAITING ? L"Waiting" : L"Playing") << std::endl;
    std::wcout << "= = = = = = =" << std::endl;
}

void WChessServer::fifo_displayTypeClient(CLIENT_STATUS status) {
    int count = 0;

    for (auto [str, client]: m_Clients) {
        // IF UNBOUND IS PASSED THEN JUST DISPLAY ALL CLIENTS
        if (status == UNBOUND){
            fifo_displayClientInfo(client);
            count++;
        } else if(client.status == status){
            fifo_displayClientInfo(client);
            count++;
        }
    }

    setTerminalColor(BOLD);
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
    setTerminalColor(DEFAULT);
}

void WChessServer::fifo_displayLobbies(){
    std::wcout << "Unimplemented..." << std::endl;
}

void WChessServer::fifo_killServer(){
    std::wcout << "Unimplemented..." << std::endl;
}

void WChessServer::fifo_killLobbies(){
    std::wcout << "Unimplemented..." << std::endl;
}


void WChessServer::fifo_displayPrivateLobbies(){
    int count = 0;
    for(auto [code, clientStr]: m_LobbyCodes){
        std::wcout << "\n- - - - - - -" << std::endl;
        std::wcout << "Private lobby code: " << convertString(code) << "\nOwner: ";
        fifo_displayClientInfo(m_Clients[clientStr]);
        std::wcout << "- - - - - - -" << std::endl;
        count++;
    } 
    setTerminalColor(BOLD);
    if(count == 0)
        std::wcout << "No client private lobbies found..." << std::endl;
    else
        std::wcout << count << " client private lobbies found..." << std::endl;
    setTerminalColor(DEFAULT);
}

void WChessServer::fifo_handleCommand(std::string command){

    if(command.compare(KILL_SERVER_CMD) == 0)
        fifo_killServer(); // Kill server 
    else if(command.compare(ALL_CLIENTS_CMD) == 0 || command.compare(ALL_CLIENT_CMD) == 0)
        fifo_displayTypeClient(UNBOUND); // Display all clients --> USING UNBOUND TO DISPLAY ALL
    else if(command.compare(WAITING_CLIENTS_CMD) == 0 || command.compare(WAIT_CLIENT_CMD) == 0)
        fifo_displayTypeClient(WAITING); // Display all waiting clients
    else if(command.compare(PLAYING_CLIENTS_CMD) == 0 || command.compare(PLAY_CLIENT_CMD) == 0)
        fifo_displayTypeClient(PLAYING); // Display all playing clients
    else if(command.compare(CLIENT_LOBBIES_CMD) == 0 || command.compare(CLIENT_LOBBY_CMD) == 0)
        fifo_displayLobbies(); // Display all client game lobbies
    else if(command.compare(CLIENT_PRIVATE_LOBBIES_CMD) == 0 || command.compare(CLIENT_PRIVATE_LOBBY_CMD) == 0)
        fifo_displayPrivateLobbies(); // Display all private lobbies and their creator / maker
    else if(command.compare(SERVER_KILL_LOBBIES_AND_THREAD) == 0 || command.compare(KILL_LOBBY_THREAD_CMD) == 0)
        fifo_killLobbies(); // this will not work as expected
    else
        std::wcout << "Unknown command --> " << convertString(command) << std::endl;
}

void WChessServer::readTerminalCommands() {
    char buffer[ONLINE_BUFFER_SIZE];
    ssize_t bytesRead;

    bytesRead = read(m_TerminalFd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the buffer
        fifo_handleCommand(std::string(buffer));
    } else if (bytesRead == -1 && errno == EAGAIN) {
        return; // Nothing to read
    }
}

void WChessServer::startServer() {
    RUNNING = true;

        //! BLOCKING ON ACCEPT ACCEPTS FIRST SOCKET AND ADDS BIND STR TO CLIENT INSTANCE - BLOCKING
    while(RUNNING)
    {

        std::vector<Client*> activeClientsTracker;
        // Runs 1 time per 10 seconds
        sendPollMsg(activeClientsTracker);
        
        // Runs for 10 seconds
        for(int curr_second = 0; curr_second < 10; curr_second++){

            // Runs 4 times a second
            for(int curr_hundreth_milisecond = 0; curr_hundreth_milisecond < 4; curr_hundreth_milisecond++){
                // Accepts initial client poll sockets
                acceptClientPollSocket();     

                // Accepts initial client game sockets
                acceptClientGameSocket(); 

                // Checks for commands send by server terminal
                if (m_TerminalFd >= 0)
                    readTerminalCommands();

                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }

            // Runs 1 time a second
            waitingClientUpdate();
        }

        // Runs 1 time per 10 seconds
        recievePollMsg(activeClientsTracker);
    }
}