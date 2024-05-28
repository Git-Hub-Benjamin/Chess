#pragma once

#define KILL_SERVER_CMD "kill"
#define ALL_CLIENTS_CMD "all-clients"
#define WAITING_CLIENTS_CMD "wait-clients" // displays all clients in waiting state
#define PLAYING_CLIENTS_CMD "play-clients" // displays all clients in playing state
#define CLIENT_LOBBIES_CMD "game-clients"  // displays the games active and the players in them
#define CLIENT_PRIVATE_LOBBIES_CMD "private-lobby-clients" // displays created private lobbies and what clients it belongs to
#define SERVER_KILL_LOBBIES_AND_THREAD "kill-lobbies" // kills all the lobbies and terminates the thread associated with it

// SHORTHAND FOR CMDS ABOVE
#define ALL_CLIENT_CMD "aclients"
#define WAIT_CLIENT_CMD "wclients"
#define PLAY_CLIENT_CMD "pclients"
#define CLIENT_LOBBY_CMD "lclients"
#define CLIENT_PRIVATE_LOBBY_CMD "privclients"
#define KILL_LOBBY_THREAD_CMD "kthlobby" // short for kill thread lobby
