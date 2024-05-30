#include "../chess.hpp"
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

#ifndef CHESSSERVER_H
#define CHESSSERVER_H


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

    enum Owner Player;
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

class Online_ChessGame{
    bool GO = false;
    std::thread game_thread;
    int client_lobbies_index;
    Client& current_turn_client;
    Client& non_current_turn_client;
    ChessGame Lobby_Game; // invokes the default constructor and called init

    void gameloop();
    bool end_turn_client_check_in();
    int recieve_non_turn_client_check_in();
    bool send_preturn_check_in(std::string&);
    bool verify_client_connection_init_turn();
    int recieve_turn_client_move(std::string& out);
    int three_second_check_on_clients(std::string&);
    void format_turn_move(std::string& in, std::wstring& move, std::wstring& to);
    void check_valid_move(ChessGame& LobbyGame, GameSqaure* movePiece, GameSqaure* moveToSquare, std::string& out);
    void convert_move_square(GameSqaure*& movesquare, GameSqaure*& moveToSquare, std::wstring& move, std::wstring& to, bool in_check);
public:
    // This means that the client selected to go first should be passed first 
    Online_ChessGame(int lobby_index, Client& cli1, Client& cli2):
        client_lobbies_index(lobby_index), current_turn_client(cli1), non_current_turn_client(cli2), lobby_status(ACTIVE){}

    enum LOBBY_STATUS lobby_status;
    // Called by Main thread only
    void start_game() { GO = !GO; }
    void init(){
        // Thread running this will wait until the main thread says it is ok to start
        while(!GO){
            std::this_thread::yield();
        }

        gameloop();

        std::wcout << "Game thread ended the game loop..." << std::endl;
    }
};




#endif