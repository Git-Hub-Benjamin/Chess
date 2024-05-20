#pragma once

#define CLIENT_TELL_SERVER_FAIL "error" // this will happen if only 1 socket can connect to the server

//* BINDING MACROS
// Clients should expect server to respond with "bind:good or fail"
#define CLIENT_ID_BIND_MSG "bind:" // + random string 
#define SERVER_CLIENT_INIT_GOOD "bind:good"
#define SERVER_CLIENT_INIT_FAIL "bind:fail"
#define BIND_INDEX_OF_START_RAND_STRING 5
#define CLIENT_ID_RAND_STRING_LEN 10
#define BIND_MSG_LENGTH (sizeof("bind:") + CLIENT_ID_RAND_STRING_LEN)

//* POLLING MACROS
#define INDEX_OF_COLON_IN_POLL_MSG 8
#define SERVER_POLL_MSG "connection-poll-report"
#define CLIENT_POLL_MSG "connection-alive:true"
#define GAME_FAIL_OTHER_PLAYER_DC "game-fail:dc"

//* PRIVATE LOBBY MACROS
#define CLIENT_LOBBY_JOIN_CODE "join-code:"
#define INDEX_AFTER_COLON_JOIN_CODE 10
#define CLIENT_CREATE_PRIVATE_LOBBY "lobby-create"
#define CLIENT_CLOSE_LOBBY "lobby-!close"
#define SERVER_CLIENT_CLOSE_LOBBY_GOOD "lobby-close-good"
#define SERVER_CREATE_LOBBY_GOOD "lobby-create-good:" // + code
#define CLIENT_INDEX_AFTER_COLON_IN_CREATE_LOBBY_CODE 18
#define SERVER_CREATE_LOBBY_BAD "lobby-create-fail"
//* Good message of joining private lobby will just match-found
#define SERVER_CLIENT_JOIN_PRIVATE_LOBBY_BAD "join-private-bad"

//* QUEUE RANDOM
#define CLIENT_JOIN_RANDOM_QUEUE "join-rand-queue"
#define SERVER_CLIENT_JOIN_RAND_QUEUE_GOOD "join-rand-queue-good"
#define CLIENT_LEAVE_RANDOM_QUEUE "leave-rand-queue"
#define SERVER_CLIENT_LEAVE_RAND_QUEUE_GOOD "leave-rand-queue-good"

//* MATCH FOUND
#define MATCH_FOUND "match-found"

#define WAITING_MSG_W "waiting: \n"
#define WAITING_MSG_P "playing: \n"
#define WAITING_MSG_FIRST_INDEX 8
#define WAITING_MSG_SECOND_INDEX 8