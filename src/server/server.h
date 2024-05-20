#ifndef CHESSSERVER_H
#define CHESSSERVER_H

enum CLIENT_STATUS{
    UNBOUND, // If the heart_beat sock still needs to be bound to the client
    WAITING,
    PLAYING
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





#endif