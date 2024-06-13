#pragma once

#include "../../Chess/chess.hpp"
#include "../../socket/socketutil.h"
#include "../client-text-graphics/textgraphic.hpp"
#include "../client-terminal-frontend/displaymanager.hpp"
#include <unistd.h>
#include <thread>
#include <sstream>
#include <csignal>
#include <atomic>
#include <poll.h> 

extern Options global_player_option;
extern int get_menu_option();
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
