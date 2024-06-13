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


enum CLIENT_STATUS{
    UNBOUND, // If the heart_beat sock still needs to be bound to the client
    WAITING,
    PLAYING,
    POLL,    // this is litterally just for the readfifo
};

enum LOBBY_STATUS{
    ACTIVE,
    QUEUE_KILL, // Self thread sets this,
    AKN_KILL // Main thread will set this and make the thread join the main thread
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
    socketTracker Game; 
    socketTracker Poll;

    // constructor
    Client(int fd, int index, std::string& str): Poll(fd, index), CLIENT_STRING_ID(str){
        status = UNBOUND;
        timeSinceUnbound = std::chrono::system_clock::now();
    } 
};

class OnlineChessGame{

protected:
    bool mainStartGame = false; // Hate doing this but this is so this doesnt start the game thread before the main thread lets it loose
    Client& currentTurnClient;
    Client& nonCurruentTurnClient;
    GAME_MODE GameMode;

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

    virtual void startGame() = 0; // To be overidden by game to be called

public:
    // This means that the client selected to go first should be passed first 
    OnlineChessGame(int lobbyIndex, GAME_MODE mode, Client& client1, Client& client2) : LobbyIndex(lobbyIndex), currentTurnClient(client1), nonCurruentTurnClient(client2), LobbyStatus(ACTIVE), GameMode(mode) {}
    
    int LobbyIndex;
    enum LOBBY_STATUS LobbyStatus;

    virtual ~OnlineChessGame() = default;

};

class StandardServerOnlineChessGame : public StandardChessGame, public OnlineChessGame  {

    // All the functions here are just for interacting with the clients

    void formateCurrTurnMove(std::string&, std::wstring&, std::wstring&);

public:

    StandardServerOnlineChessGame(int lobbyIndex, GAME_MODE mode, Client& client1, Client& client2) 
        : OnlineChessGame(lobbyIndex, mode, client1, client2) {}
    
    void startGame() override;
    void setStart() { mainStartGame = true; }

};

