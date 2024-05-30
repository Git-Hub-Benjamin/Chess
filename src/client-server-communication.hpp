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
// "match-found,opponent:bind___str,playerNum:_"
#define MATCH_FOUND "match-found"
#define CLIENT_EXTRACT_OPPONENT_BIND_STR 21
#define CLIENT_EXTRACT_OPPONENT_BIND_STR_LEN 10
#define CLIENT_RDY_FOR_MATCH ":match-ready" // before : will be the client bind string that is sending the match ready
#define SERVER_CLIENT_ACK_MATCH_RDY "match-start" // after colon will be P1 or P2
#define CLIENT_INDEX_AFTER_COLON_MATCH_START 12
#define SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT "match-bad-you"
#define SERVER_CLIENT_ACK_MATCH_RDY_BAD_OTHER_FAULT "match-bad-other"

//* MATCH COMMUNICATION

// Before each turn, the game status will be sent to each player
#define GAMESTATUS_GAMEOVER_DC "opponent-dc" // other player disconnected
#define GAMESTATUS_SURRENDER "gamestatus-surrender"    // other player surrender

// if you got this and it WAS and its your turn then you lose bc you got checkmated
// if you got this and its NOT your turn then you win bc you just did the move
// that got them checkmated
#define GAMESTATUS_CHECKMATE "gamestatus-checkmate"   
// like checkmate if you get this and its your turn you are in check
// if you get this and its not your turn you just put the other player in check 
#define GAMESTATUS_CHECK "gamestatus-check"
#define GAMESTATUS_ALL_GOOD "gamestatus-good"

#define CLIENT_NON_TURN_CHECK_IN "non-turn-check-in"
#define CLIENT_RDY_FOR_NEXT_TURN "next-turn-ready"

// To the player whos not their turn, after the other player has done their turn
// gamestat:(-1,0,1, 2)\n // -1 Disconnected, 0 Surrender, 1 Checkmate, 2 (alive but check), 3 alive
// move:( )\n
// to:( )\n

#define SERVER_CLIENT_INDEX_MOVE_PT_1 6
#define SERVER_CLIENT_INDEX_MOVE_PT_2 7

#define SERVER_CLIENT_INDEX_TO_PT_1 12
#define SERVER_CLIENT_INDEX_TO_PT_2 13

// To the player whos turn it is

// From client to server:
// "move:()to:()"
#define CLIENT_MOVE_INDEX_AFTER_COLON 5 // to 6
#define CLIENT_TO_INDEX_AFTER_COLON 10 // to 11

// After the server verifys the move it will send back this to the client
// "move:(0,1)\ncheck:(0,1)"

#define SERVER_CLIENT_INDEX_MOVE_VALID 5
#define SERVER_CLIENT_INDEX_CHECK_OR_NOT 13

#define SERVER_CLIENT_VERIFY_MOVE_VALID "client-move-valid"
#define SERVER_CLIENT_VERIFY_MOVE_INVALID "client-move-invalid"
#define SERVER_CLIENT_VERIFY_MOVE_INVALID_CHECK "client-move-check"

#define WAITING_MSG_W "waiting: \n"
#define WAITING_MSG_P "playing: \n"
#define WAITING_MSG_FIRST_INDEX 8
#define WAITING_MSG_SECOND_INDEX 8

//* Errors
// This should actually never happen, but there is the smallest chance that it could
#define SERVER_CLIENT_MAX_CAPACITY "server-max-capacity"