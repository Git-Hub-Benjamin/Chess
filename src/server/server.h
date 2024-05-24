#include "../chess.hpp"

#ifndef CHESSSERVER_H
#define CHESSSERVER_H

enum CLIENT_STATUS{
    UNBOUND, // If the heart_beat sock still needs to be bound to the client
    WAITING,
    PLAYING
};

enum LOBBY_STATUS{
    ACTIVE,
    QUEUE_KILL, // Self thread sets this,
    AKN_KILL // Main thread will set this and make the thread join the main thread
};

enum GAME_MODE{
    NONE_SELECTED,
    CREATE_ROOM_ENTER_CODE,
    QUEUE_RANDOM
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

    enum GAME_MODE client_selected_game_move = NONE_SELECTED;
    enum CLIENT_STATUS status;
    socketTracker Game; 
    socketTracker Poll;

    // constructor
    Client(int fd, int index, std::string& str): Poll(fd, index), CLIENT_STRING_ID(str){
        status = UNBOUND;
        timeSinceUnbound = std::chrono::system_clock::now();
    } 
};

struct Online_ChessGame{

    Online_ChessGame(std::thread* thread, int lobby_index, Client& cli1, Client& cli2):
    game_thread(thread), client_lobbies_index(lobby_index), client1(cli1), client2(cli2){
        lobby_status = ACTIVE;
    }
    std::thread* game_thread;
    int client_lobbies_index;
    Client& client1;
    Client& client2;
    enum LOBBY_STATUS lobby_status;

    ChessGame game; // invokes the default constructor and called init
};




#endif