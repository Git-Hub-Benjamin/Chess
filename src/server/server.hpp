#pragma once

#include "../Chess/chess.hpp"
#include <stdio.h>
#include <string>
#include <pthread.h> // may user thread
#include <thread>
#include <mutex>
#include <time.h>
#include <unistd.h>
#include <random>
#include <netinet/in.h>
#include <fcntl.h>
#include <unordered_map>
#include <memory>
#include <csignal>
#include <execinfo.h>
#include <atomic>
#include <sys/stat.h>

#define MAX_CLIENT 8



enum CLIENT_STATUS{
    UNBOUND, // If the heart_beat sock still needs to be bound to the client
    WAITING,
    PLAYING,
    DISCONNECTED, // this is set when the poll socket sees that a client DC, this will be set if current status of a client is playing so the game thread knows this client DC-ed
    DISCONNECTED_KILL, // set by game thread when client is safe to close
    POLL    // this is litterally just for the readfifo
};

enum LOBBY_STATUS{
    ACTIVE,
    QUEUE_KILL, // Self thread sets this,
};

// Because LOBBY_STATUS must be decalred first
extern void queue_lobby_close(enum LOBBY_STATUS& stat);

enum JOIN_MODE {
    NONE_SELECTED,
    CREATE_ROOM_ENTER_CODE,
    QUEUE_RANDOM
};

enum GAME_MODE {
    STANDARD_CHESSGAME
};

class socketTracker{
public:

    int fd;
    int index;
    socketTracker(){}
    socketTracker(int fd, int index): fd(fd), index(index) {}
};


struct Client{
    std::chrono::_V2::system_clock::time_point timeSinceUnbound;
    std::string CLIENT_STRING_ID;
    std::string RES_TO_CLIENT_WAITING;

    enum Owner Player;
    GAME_MODE clientGameMode = STANDARD_CHESSGAME; // DEFAULT
    JOIN_MODE clientJoinMode = NONE_SELECTED;
    CLIENT_STATUS status;
    int gameFd = -1; 
    int pollFd = -1;

    // constructor
    Client(int fd, std::string bind): pollFd(fd), CLIENT_STRING_ID(bind){
        status = UNBOUND;
        timeSinceUnbound = std::chrono::system_clock::now();
    } 

    Client(){}
};

class OnlineChessGame{

protected:
    bool mainStartGame = false; // Hate doing this but this is so this doesnt start the game thread before the main thread lets it loose
    Client& currentTurnClient;
    Client& nonCurruentTurnClient;
    GAME_MODE GameMode;

    bool sendCurrTurnValidatityOfMove(std::string&);

    // Call when need to close this lobby, after calling this return so the thread dies
    void closeSelfLobby();

    // Marks a client as a certain status so main thread can clean up, also sends a gamestatus DC to client
    void disconnectClient(Client&);

    // Gracefully ends a game for a client by sending msg and setting status to waiting
    void endGameForClient(Client&, std::string);

    // To be called before ANY communication between any game socket in game
    bool checkClientConnection();

    // Start of Game make sure both send match start
    bool initClientConnection(); 

    // Utility function, makes both clients send msg within 3 seconds
    // 0 Both responded
    // 1 Only currTurnClient Responded
    // 2 Only nonCurrTurnClient Responded
    int threeSecVerifyClientTimeout(std::string&); 

    // Send preTurn check in msg to both clients about state of game
    bool sendPreTurnCheckIn(std::string& msg); 

    // before waiting to listen for curr turn client listen for specific check in from non turn client
    // -1 - Server fault
    // 0 -- Good
    // 1 -- Client fault
    int nonTurnSpecificClientCheckIn(); 

    // blocking, waits for curr turn client to send turn / move
    // -1 - Server fault
    // 0 -- Good
    // 1 -- Client fault
    int recieveCurrTurnMove(std::string&);

    // similar to pre turn check in just end of turn check in
    bool endOfTurnClientsCheckIn(); 

    // To be overidden by game to be called
    virtual void startGame() = 0; 

public:
    // This means that the client selected to go first should be passed first 
    OnlineChessGame(GAME_MODE mode, Client& client1, Client& client2) : currentTurnClient(client1), nonCurruentTurnClient(client2), LobbyStatus(ACTIVE), GameMode(mode) {}
    
    LOBBY_STATUS LobbyStatus;

    virtual ~OnlineChessGame() = default;

};

class StandardServerOnlineChessGame : public StandardChessGame, public OnlineChessGame  {

    // All the functions here are just for interacting with the clients

    void formateCurrTurnMove(std::string&, std::wstring&, std::wstring&);

public:

    StandardServerOnlineChessGame(GAME_MODE mode, Client& client1, Client& client2) 
        : OnlineChessGame(mode, client1, client2) {}
    
    void startGame() override;
    void setStart() { mainStartGame = true; }

};

class WChessServer {

    //* Client Data
    std::unordered_map<std::string, Client> m_Clients;

    //* Server data
    std::vector<OnlineChessGame> m_Lobbies; 
    std::unordered_map<std::string, std::string> m_LobbyCodes; // Lobby Code --> Client String

    //* Server Fds
    const int m_ServerPollFd;
    const int m_ServerGameFd;
    const int m_TerminalFd; // May not be used

    //* Server state
    int m_ClientOnlineCount = 0;
    int m_ClientPlayingCount = 0;
    int m_ClientWaitingCount = 0;
    std::atomic_bool RUNNING;
    std::mutex clientAccess;

public:

    WChessServer(int pollFd, int gameFd, int fifoTerminalFd) : m_ServerPollFd(pollFd), m_ServerGameFd(gameFd), m_TerminalFd(fifoTerminalFd) {}
    void startServer();

private:

    // Main loop
    void sendPollMsg(std::vector<Client*>&);
    void acceptClientPollSocket();
    void acceptClientGameSocket();
    bool clientBindSockets(std::string);
    void recievePollMsg(std::vector<Client*>& check_active);
    std::string createPrivateRoom(std::string str);
    void closeClientPrivateRooms(std::string str);
    bool joinClientPrivateRoom(std::string code);
    void newOnlineGame(Client& client1, Client& client2);
    void dispatchClients();
    void waitingClientUpdate();
    void randomQueue();
    Owner fiftyFiftyTurnGenerator();
    void addClient(int, std::string&);

    // Fifo terminal
    void readTerminalCommands();
    void fifo_displayPrivateLobbies();
    void fifo_handleCommand(std::string);
    void fifo_displayLobbies();
    void fifo_killServer();
    void fifo_killLobbies();
    void fifo_displayTypeClient(CLIENT_STATUS);
    void fifo_displayClientInfo(Client&);
};


