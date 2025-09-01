#pragma once

#include "../../socket/socketutil.h"
#include "../client-text-graphics/textgraphic.hpp"
#include "../client-terminal-frontend/displaymanager.hpp"
#include "../tui/clientoption.hpp"


extern Options global_player_option;
extern std::string ONLINE_PLAYER_ID;

struct JOIN_GAME_INFO {
    // -1 - Back
    // 0 - Server Error
    // 1 - Client Error
    // 2 - All Good!
    int joinState = 2;
    Player myPlayerNum;
    std::string opposingPlayerStr;

    JOIN_GAME_INFO(Player player, std::string str) : myPlayerNum(player), opposingPlayerStr(str) {}
    JOIN_GAME_INFO(int res) : joinState(res) {}
    JOIN_GAME_INFO(){}
};

extern JOIN_GAME_INFO randomQueue(int fd);
extern JOIN_GAME_INFO joinPrivateLobby(int fd);
extern JOIN_GAME_INFO createPrivateLobby(int fd);


#ifdef _WIN32
    typedef SOCKET socket_t;  // Windows uses SOCKET type
#else
    typedef int socket_t;     // Unix-like systems use int
#endif

#ifdef _WIN32
    typedef const char* send_type;
#else
    typedef const void* send_type;
#endif

#ifdef _WIN32
    typedef char* buf;
#else
    typedef const void* buf;
#endif

extern int closeSocket(socket_t);
extern int receiveData(socket_t, send_type, size_t, int);
extern int sendData(socket_t, send_type, size_t);


