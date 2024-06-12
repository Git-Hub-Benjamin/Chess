#include "./online-mode.hpp"


static void server_sent_zero_bytes_fatal_error(){
    set_terminal_color(RED);
    std::wcout << "Fatal Error: "; 
    set_terminal_color(DEFAULT);
    std::wcout << "Server stopped responding..." << std::endl;
    exit(EXIT_FAILURE);
}

// True  --> No error
// False --> Error
static bool take_moves_and_send(int fd, std::string move, std::string to){
    std::string send_to_server = "move:";
    send_to_server += move;
    send_to_server += "to:";
    send_to_server += to;

    // Example:
    // move:b2to:b4

    std::wcout << "Sending: " << convertString(send_to_server) << ", to server" << std::endl;
    if(send(fd, (void*)send_to_server.c_str(), send_to_server.length(), 0) < 0)
        return false;
    return true;
}

// -2 Server error
// -1 Puts self in check, so you cant make this move
// 0  Invalid move
// 1  Valid move
static int server_said_valid_move(int fd){
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), 0);

    if(bytes_read <= 0)
        return -2;

    buffer[bytes_read] = '\0';
    std::string res_from_server(buffer);

    if(res_from_server.compare(SERVER_CLIENT_VERIFY_MOVE_VALID) == 0)
        return 1; // Valid move
    else if(res_from_server.compare(SERVER_CLIENT_VERIFY_MOVE_INVALID) == 0)
        return 0;
    return -1;
}

// True  --> Means move was recieved from server
// False --> Opponent DC-ed, we dont have a move to makeMove with
static bool notTurn_getMove(int fd, std::wstring& move, std::wstring& moveTo){
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    // This will block until the other player completes their turn

    std::wcout << "Blocking on the recv for the waiting non turn player" << std::endl;

    // Formatted: "move:()to:()"
    int byte_read = recv(fd, (void*)buffer, sizeof(buffer), 0);

    // So this will return the other players move and to, but if the player DC-ed
    // instead it will return an error message that they DC-ed

    buffer[byte_read] = '\0';
    std::string res_from_server(buffer);

    if(res_from_server.compare(GAMESTATUS_GAMEOVER_DC) == 0)
        return false;

    move += res_from_server[CLIENT_MOVE_INDEX_AFTER_COLON];
    move += res_from_server[CLIENT_MOVE_INDEX_AFTER_COLON + 1];
    moveTo += res_from_server[CLIENT_TO_INDEX_AFTER_COLON];
    moveTo += res_from_server[CLIENT_TO_INDEX_AFTER_COLON + 1];

    std::wcout << "Recieved from server for non turn, Move: " << move << ", MoveTo: " << moveTo << std::endl;


    return true;
}

// False -- Error
// True  -- Good
static bool verify_player_server_connection(int fd){
    if(send(fd, SERVER_CLIENT_ACK_MATCH_RDY, sizeof(SERVER_CLIENT_ACK_MATCH_RDY), 0) < 0)
        return false;
    return true;
}

// Gameover  - 0
// Surrender - 1
// CheckMate - 2
// Check     - 3
// All good  - 4 
static int turnly_check_in(int fd){
    
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(fd, (void*)buffer, sizeof(buffer), 0);
    
    if(byte_read <= 0)
        server_sent_zero_bytes_fatal_error();
    
    buffer[byte_read] = '\0';
    std::string res_from_server(buffer);

    if(res_from_server.compare(GAMESTATUS_GAMEOVER_DC) == 0)
        return 0; // again idk when this would occur
    
    if(res_from_server.compare(GAMESTATUS_SURRENDER) == 0)
        return 1;

    if(res_from_server.compare(GAMESTATUS_CHECKMATE) == 0)
        return 2;
    
    if(res_from_server.compare(GAMESTATUS_CHECK) == 0)
        return 3;

    return 4;
}

static bool nonTurnSpecificCheckIn(int fd){
    if(send(fd, CLIENT_NON_TURN_CHECK_IN, sizeof(CLIENT_NON_TURN_CHECK_IN), 0) < 0)
        return false;
    return true;
}

void game_loop(int game_fd, enum Owner myPlayerNum, std::string otherPlayerStr){

    std::wcout << "In client game_loop" << std::endl;

    while (true) {}

    // ChessGame Game;
    // Game.GameOptions = global_player_option;
    // bool& game_gameover = Game.gameover;

    // if(!verify_player_server_connection(game_fd))
    //     return;
    
    // std::wcout << "= = = = =\nMe: " << convertString(ONLINE_PLAYER_ID) << ", Player: " << (myPlayerNum == PONE ? "Player one" : "Player two") << std::endl;
    // std::wcout << "Other: " << convertString(otherPlayerStr) << ", Player: " << (myPlayerNum == PONE ? "Player two" : "Player one") << "\n= = = = =" << std::endl;

    // while(!game_gameover){

    //     set_terminal_color(GREEN);
    //     std::wcout << "Game Current Turn: " << Game.currentTurn << ", myPlayerNum: " << myPlayerNum << std::endl;
    //     set_terminal_color(DEFAULT);
        
    //     //* Regardless of the turn, you get a pre turn check in from the server

    //     bool in_check = false;

    //     std::wcout << "Recieving the turnly check in..." << std::endl;
    //     int res = turnly_check_in(game_fd);
    //     std::wcout << "Turnly check in recieved..." << std::endl;
    //     if(res < 3){
    //         // End game
    //         std::wstring end_game_msg;
    //         if(res == 0)
    //             end_game_msg = L"Gameover, connection problem.";
    //         else if(res == 1)
    //             if(Game.currentTurn == myPlayerNum)
    //                 end_game_msg = L"Gameover, You Lose! You surrendered.";
    //             else //! Honestly idk if these should be the other way around
    //                 end_game_msg = L"Gameover, You Win! They surrendered.";
    //         else
    //             if(Game.currentTurn == myPlayerNum)
    //                 end_game_msg = L"Gameover, Checkmate, You Win!";
    //             else //! Honestly idk if these should be the other way around
    //                 end_game_msg = L"Gameover, Checkmate, You lose!";
    //         std::wcout << end_game_msg << std::endl;
    //         return;
    //     }else if(res == 3)
    //         in_check = true; // Check


    //     print_board(Game);

    //     if(myPlayerNum == Game.currentTurn){

    //         while(true){

    //             int res;

    //             std::wstring move;
    //             std::wstring moveTo;
    //             std::wstring ret_msg;

    //             if(!in_check){
    //                 std::wcout << "Your move: ";
    //                 res = getMove(move);
    //                 if(res != 0){ 
    //                     // handle
    //                     if(res == 1){ // 1 --> Option change, 
    //                         // QUIT
    //                     }else{
    //                         continue; // 2 --> Invalid input
    //                     }
    //                 }

    //                 validateMovePiece(Game, *moveConverter(Game, move), ret_msg);

    //                 if(ret_msg.length() != 0){
    //                     std::wcout << ret_msg << std::endl;
    //                     continue;
    //                 }
    //             }else{
    //                 move = L"KI";
    //             }

    //             if(Game.GameOptions.moveHighlighting){
    //                 std::vector<GameSqaure*>* squaresPieceCanMoveTo;
    //                 GameSqaure* movePiece = moveConverter(Game, move);
    //                 squaresPieceCanMoveTo = get_move_to_squares(Game, *movePiece);
    //                 if(squaresPieceCanMoveTo->size() > 0) 
    //                     print_board_with_moves(Game, *movePiece, *squaresPieceCanMoveTo);
    //                 else{
    //                     std::wcout << "No valid moves with this piece." << std::endl;
    //                     continue;
    //                 }
    //             }

    //             std::wcout << "Move to: ";
    //             res = getMove(moveTo);
    //             if(res != 0){
    //                 // hanlde
    //                 // handle
    //                 if(res == 1){ // 1 --> Option change, 
    //                     // QUIT
    //                 }else{
    //                     continue; // 2 --> Invalid input
    //                 }
    //             }

    //             validateMoveToPiece(Game, *moveConverter(Game, moveTo), ret_msg);
            
    //             if(ret_msg.length() != 0){
    //                 std::wcout << ret_msg << std::endl;
    //                 continue;
    //             }

    //             std::wcout << "Basic validation was made on the move and to, now we are going to send the move to the server" << std::endl;

    //             // Basic checks are made on the piece and move to piece / square
    //             // The server will make verifyMove and other checks for safety purposes

    //             take_moves_and_send(game_fd, convertWString(move), convertWString(moveTo));

    //             std::wcout << "Sent to server the move, now we wait for the result from the server" << std::endl;
                
    //             res = server_said_valid_move(game_fd);
    //             if(res == -1){
    //                 std::wcout << "This puts you in check..." << std::endl;
    //                 continue;
    //             }else if(res == 0){
    //                 std::wcout << "Piece cannot reach that position..." << std::endl;
    //                 continue;
    //             }

    //             std::wcout << "The move was valid, now we will make the move" << std::endl;

    //             // Valid move

    //             if(makeMove(Game, *moveConverter(Game, move), *moveConverter(Game, moveTo)) == 0){
    //                 std::wcout << "Piece moved." << std::endl;
    //             }else{
    //                 std::wcout << "Piece taken." << std::endl;
    //             }

    //             break;
    //         }

    //     }else{

    //         std::wcout << "Sending server non check in" << std::endl;

    //         if(!non_turn_check_in(game_fd))
    //             // hanlde send socket error

    //         std::wcout << "Other players turn..." << std::endl;

    //         // Get other players move from last turn

    //         std::wstring move;
    //         std::wstring moveTo;

    //         if(notTurn_getMove(game_fd, move, moveTo)){ 
    //             // makeMove() with the other players move

    //             if(makeMove(Game, *moveConverter(Game, move), *moveConverter(Game, moveTo)) == 0){
    //                 std::wcout << "Opponent moved a piece." << std::endl;
    //             }else{
    //                 std::wcout << "Opponent took a piece." << std::endl;
    //             }
    //         }

    //         // If there is no getMove because the opponent dc-ed it will handle it on the next loop

    //     }

    //     // The server needs to wait for a recv once before sending pre turn check in
    //     // to ad here to client -> server communication standards

    //     std::wcout << "Sending to the server the end check in" << std::endl;

    //     // After it sends this it will be waiting for the pre turn check in
    //     if(send(game_fd, (void*)CLIENT_RDY_FOR_NEXT_TURN, sizeof(CLIENT_RDY_FOR_NEXT_TURN), 0) < 0)
    //         server_sent_zero_bytes_fatal_error();

    //     std::wcout << "Sent to the server the end check in" << std::endl;

        // Swap turns
        //Game.currentTurn = (Game.currentTurn == PONE ? PTWO : PONE);

//    }
}