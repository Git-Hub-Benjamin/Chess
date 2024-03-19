#ifndef CHESSSERVER_H
#define CHESSSERVER_H

enum CLIENT_STATUS{
    WAITING,
    PLAYING
};

class ChessClient{
public:

    int client_socket_fd;
    int client_index;

    ChessClient(int fd, int index): client_socket_fd(fd), client_index(index) {}
};

struct Client{
    enum CLIENT_STATUS status;
    ChessClient client; // pointer?

    // constructor
    Client(int fd, int index): client(fd, index){
        status = WAITING;
    }
};





#endif