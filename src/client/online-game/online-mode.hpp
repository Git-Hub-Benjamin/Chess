#pragma once

#include "../../chess.hpp"
#include "../../server/socket/socketutil.h"
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

int rand_queue_wait(int fd);
int join_private_lobby(int fd);
int create_private_lobby(int fd);
void game_loop(int game_fd, enum Owner myPlayerNum, std::string otherPlayerStr);