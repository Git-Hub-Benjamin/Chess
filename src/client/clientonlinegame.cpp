#include "../chess.hpp"
#include "../server/socket/socketutil.h"
#include <unistd.h>
#include <thread>
#include <sstream>
#include <csignal>
#include <atomic>
 

extern Options global_player_option;
extern int get_menu_option();
static std::wstring ONLINE_PLAYER_ID;
static std::string bind_str;


void online_menu(){
    std::wcout << "\n\n\n\n\n";
	std::wcout << "\t\t\t|=====================================|\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|         W-CHESS - Online mode       |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    1. Random queue                  |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    2. Create private room           |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    3. Join private room             |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|    4. Back - PlayerId: " << ONLINE_PLAYER_ID << "   |\n";
    std::wcout << "\t\t\t|                                     |\n";
    std::wcout << "\t\t\t|=====================================|\n";
	std::wcout << "\n\n\n--> ";
}

static std::atomic_bool ONLINE_QUIT(false);

void poll_to_server(int poll_fd){
    int count = 1;
    while(!ONLINE_QUIT){

        std::this_thread::sleep_for(std::chrono::seconds(1));

        char buffer[sizeof(SERVER_POLL_MSG)] = {0};
        
        int bytes_read = recv(poll_fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
        
        if(bytes_read == 0)
            break; // Means that the server closed out connection
        
        if(bytes_read < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
            continue; // Means that there was no data available

        buffer[bytes_read] = '\0';
        std::string msg(buffer);

        send(poll_fd, (void*)CLIENT_POLL_MSG, sizeof(CLIENT_POLL_MSG), 0);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void server_sent_zero_bytes_fatal_error(){
    set_terminal_color(RED);
    std::wcout << "Fatal Error: "; 
    set_terminal_color(DEFAULT);
    std::wcout << "Server stopped responding..." << std::endl;
    exit(EXIT_FAILURE);
}

// Who ever gets P1, they go first
// Server sends P1 or P2
enum Owner init_player_num_from_server(int fd){
    char buffer[10] = {0};
    recv(fd, (void*)buffer, sizeof(buffer), 0);
    if(buffer[1] == '1')
        return PONE;
    return PTWO;
}

// True  --> No error
// False --> Error
bool take_moves_and_send(int fd, std::string move, std::string to){
    std::stringstream send_to_server;
    // server is going to split on "\n"
    send_to_server << "move:" << move << "\nto:" << to;
    if(send(fd, (void*)send_to_server.str().c_str(), send_to_server.str().length(), 0) < 0)
        return false;
    return true;
}

// -1 Puts self in check, so you cant make this move
// 0  Invalid move
// 1  Valid move
int server_said_valid_move(int fd){
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), 0);

    if(bytes_read <= 0)
        server_sent_zero_bytes_fatal_error();

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
bool notTurn_getMove(int fd, std::wstring& move, std::wstring& moveTo){
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    // This will block until the other player completes their turn

    int byte_read = recv(fd, (void*)buffer, sizeof(buffer), 0);

    // So this will return the other players move and to, but if the player DC-ed
    // instead it will return an error message that they DC-ed

    buffer[byte_read] = '\0';
    std::string res_from_server(buffer);

    if(res_from_server.compare(GAMESTATUS_GAMEOVER_DC) == 0)
        return false;

    move   = convertString(res_from_server.substr(SERVER_CLIENT_INDEX_MOVE_PT_1, SERVER_CLIENT_INDEX_MOVE_PT_2));
    moveTo = convertString(res_from_server.substr(SERVER_CLIENT_INDEX_TO_PT_1, SERVER_CLIENT_INDEX_TO_PT_2));

    return true;
}

// true --> GOOD
// false --> SOMETHING WENT WRONG
bool verify_player_server_connection(int fd){
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    std::string match_rdy = bind_str + CLIENT_RDY_FOR_MATCH;
    send(fd, match_rdy.c_str(), match_rdy.length(), 0);
    int byte_read = recv(fd, buffer, sizeof(buffer), 0);
    
    if(byte_read <= 0)
        exit(EXIT_FAILURE); // FATAL ERROR, IDK WHEN THAT WOULD HAPPEN, SERVER SHOULD NEVER BE DOWN

    buffer[byte_read] = '\0';
    std::string res(buffer);
    if(res.compare(SERVER_CLIENT_ACK_MATCH_RDY) != 0){
        if(res.compare(SERVER_CLIENT_ACK_MATCH_RDY_BAD_PERSONAL_FAULT))
            std::wcout << "Something went wrong with your connection to the server..." << std::endl;
        else
            std::wcout << "Something went wrong with the other players connection to the server..." << std::endl;
        return false;
    }

    std::wcout << "Both players successfully connected to lobby, Starting game..." << std::endl;
    return true;
}

// Gameover -  0
// Surrender - 1
// CheckMate - 2
// All good Make move - 4 --> 4 bc consistency with other function
int your_turn_server_lobby_pre_check(int fd){
    
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

    return 3;
}

void game_loop(int game_fd){
    
    if(!verify_player_server_connection(game_fd))
        return;

    enum Owner myPlayerNum = init_player_num_from_server(game_fd);
    
    ChessGame Game;
    Game.GameOptions = global_player_option;
    bool& game_gameover = Game.gameover;

    while(!game_gameover){
        
        //! This is where things get very complicated, What if during the game,
        //! A client DCs, on the server the poll socket will free the memory for the client
        //! Making any checks to the memory at the client will cause a seg fault
        //! This means that if a client is in a game and the poll socket stops responding,
        //! We need to find out a way to safely close everything, maybe if the poll socket sees inactivity
        //! It will mark it to be killed instead of out right killing it
        
        // ask for move
        // basic validation char & string validation
        // send to server for move validation
        // see if server said valid move, if yes then complete move locally
        // if no then redo, end turn after
        print_board(Game);

        if(myPlayerNum == Game.currentTurn){

            while(true){

                int res;

                std::wstring move;
                std::wstring moveTo;
                std::wstring ret_msg;

                std::wcout << "Your move: ";
                res = getMove(move);
                if(res != 0){ 
                    // handle
                    if(res == 1){ // 1 --> Option change, 
                        // QUIT
                    }else{
                        continue; // 2 --> Invalid input
                    }
                }

                validateMovePiece(Game, *moveConverter(Game, move), ret_msg);
                
                if(ret_msg.length() != 0){
                    std::wcout << ret_msg << std::endl;
                    continue;
                }

                std::wcout << "Move to: ";
                res = getMove(moveTo);
                if(res != 0){
                    // hanlde
                    // handle
                    if(res == 1){ // 1 --> Option change, 
                        // QUIT
                    }else{
                        continue; // 2 --> Invalid input
                    }
                }

                validateMoveToPiece(Game, *moveConverter(Game, moveTo), ret_msg);

                if(ret_msg.length() != 0){
                    std::wcout << ret_msg << std::endl;
                    continue;
                }

                // Basic checks are made on the piece and move to piece / square
                // The server will make verifyMove and other checks for safety purposes

                take_moves_and_send(game_fd, convertWString(move), convertWString(moveTo));
                
                res = server_said_valid_move(game_fd);
                if(res == -1){
                    std::wcout << "This puts you in check..." << std::endl;
                    continue;
                }else if(res == 0){
                    std::wcout << "Piece cannot reach that position..." << std::endl;
                    continue;
                }

                if(makeMove(Game, *moveConverter(Game, move), *moveConverter(Game, moveTo)) == 0){
                    std::wcout << "Piece moved." << std::endl;
                }else{
                    std::wcout << "Piece taken." << std::endl;
                }
                
            }

        }else{

            std::wcout << "Other players turn..." << std::endl;

            // Get other players move from last turn

            std::wstring move;
            std::wstring moveTo;

            if(notTurn_getMove(game_fd, move, moveTo)){ 
                // makeMove() with the other players move

                if(makeMove(Game, *moveConverter(Game, move), *moveConverter(Game, moveTo)) == 0){
                    std::wcout << "Opponent moved a piece." << std::endl;
                }else{
                    std::wcout << "Opponent took a piece." << std::endl;
                }
            }

            // If there is no getMove because the opponent dc-ed it will handle it on the next loop

        }

        // Swap turns
        Game.currentTurn = Game.currentTurn == PONE ? PTWO : PONE;
    }
}

void enter_private_lobby_code_menu(){
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Enter lobby code: " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 2 line
    std::wcout << L"\033[18C";  // Move cursor right 18 columns
    std::wcout.flush();
}

void display_private_lobby_code_menu(std::wstring& str){
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Lobby Code: " << str << std::endl;
    std::wcout << L"--> " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 2 line
    std::wcout << L"\033[4C";  // Move cursor right  columns
    std::wcout.flush();
}

void display_rand_queue_menu(){
    std::wcout << L"\n\n= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"(!back) to go back" << std::endl;
    std::wcout << L"Searching for another players..." << std::endl;
    std::wcout << L"--> " << std::endl;
    std::wcout << L"= = = = = = = = = = = = = = = = = = = = = = = = = = = =" << std::endl;
    std::wcout << L"\033[2A";  // Move cursor up 5 line
    std::wcout << L"\033[4C";  // Move cursor right 4 columns
    std::wcout.flush();
}

void join_private_lobby(int fd){
    while(true){

        enter_private_lobby_code_menu();
        std::wstring lobby_code;
        std::wcin >> lobby_code;

        if(lobby_code.compare(L"!back") == 0)
            break;
        
        if(lobby_code.length() != 5)
            continue;    
    
        // Send to server to see if valid code
        std::string build_msg = CLIENT_LOBBY_JOIN_CODE + convertWString(lobby_code);
        send(fd, (void*)build_msg.c_str(), build_msg.length(), 0);

        // Recieve response from server
        char buffer[ONLINE_BUFFER_SIZE] = {0};
        int bytes_recieved = recv(fd, (void*)buffer, sizeof(buffer), 0);
        buffer[bytes_recieved] = '\0';
        std::string res_from_server(buffer);
        if(bytes_recieved <= 0 || res_from_server.compare(MATCH_FOUND) != 0){
            std::wcout << L"\033[2B\033[G" << std::flush; // Move down 2 and to the beggining
            set_terminal_color(RED);
            std::wcout << "No lobby found with that code" << std::endl;
            set_terminal_color(DEFAULT);
            continue;
        }

        std::wcout << L"\033[2B\033[G" << std::flush; // Move down 2 and to the beggining
        // Match found
        game_loop(fd);
        break;

    }
}

void signal_handler(int sig){}

void display_code_and_wait(std::wstring& str, std::atomic_bool& stop){
    
    signal(SIGUSR1, signal_handler);

    while (stop == false)
    {
        display_private_lobby_code_menu(str);
        std::wstring input;
        std::wcin >> input;

        if(input.compare(L"!back") != 0)
            continue;

        stop = true;
    }  
}


void queue_rand_and_wait(bool& back_to_menu){
    
    signal(SIGUSR1, signal_handler);

    while (true)
    {
        display_rand_queue_menu();
        std::wstring input;
        std::wcin >> input;

        if(input.compare(L"!back") != 0)
            continue;

        // Exit thread gracefully
        back_to_menu = true;
        break;
    }  
}

int rand_queue_wait(int fd){
    // Tell server to join the random queue
    send(fd, (void*)CLIENT_JOIN_RANDOM_QUEUE, sizeof(CLIENT_JOIN_RANDOM_QUEUE), 0);

    // Receive response from the server
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), 0);
    
    // Format
    buffer[bytes_read] = '\0';
    std::string res_from_server(buffer);
    if(bytes_read <= 0 || res_from_server.compare(SERVER_CLIENT_JOIN_RAND_QUEUE_GOOD) != 0){
        
        // Chance that when joining the random queue you could be queued immediently, this happened to me
        if(res_from_server.compare(MATCH_FOUND) != 0)
        {
            std::wcout << "Error joining random queue..." << std::endl;
            return 1;
        }

        //! TEMP
        set_terminal_color(RED);
        std::wcout << "MATCH FOUND = WAITING" << std::endl;
        set_terminal_color(DEFAULT);
        while(true){}
        
        game_loop(fd);
        return 0;
        
    }
    
    bool back_to_menu_no_queue_rand = false;
    std::thread display_waiting_rand_queue_t(queue_rand_and_wait, std::ref(back_to_menu_no_queue_rand));
    
    // Waiting to be connected with another client
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
        buffer[bytes_read] = '\0';
        std::string res_from_server(buffer);        
        if(bytes_read < 0){
            if(errno != EAGAIN || errno != EWOULDBLOCK){
                // Means a genuine error occurred
                std::wcout << "Error occurred in socket" << std::endl;
                exit(EXIT_FAILURE);
            }
        }else{
            // Got data from the server
            if(res_from_server.compare(MATCH_FOUND) != 0){
                std::wcout << "Error trying to find another player to play..." << std::endl;
                return 1;
            }

            // Match found

            pthread_kill(display_waiting_rand_queue_t.native_handle(), SIGUSR1);
            display_waiting_rand_queue_t.detach();
            
            //! TEMP
            set_terminal_color(RED);
            std::wcout << "MATCH FOUND = WAITING" << std::endl;
            set_terminal_color(DEFAULT);
            while(true){}
            
            game_loop(fd);
            return 0;
        }

        if(back_to_menu_no_queue_rand){
            display_waiting_rand_queue_t.join();  
            send(fd, (void*)CLIENT_LEAVE_RANDOM_QUEUE, sizeof(CLIENT_LEAVE_RANDOM_QUEUE), 0);
            return 1;
        }

    }
}

int create_private_lobby(int fd){
    // Tell server that we want to create a private lobby
    send(fd, (void*)CLIENT_CREATE_PRIVATE_LOBBY, sizeof(CLIENT_CREATE_PRIVATE_LOBBY), 0);

    std::wcout << "Lobby code requested from server..." << std::endl;

    // Receive response from server
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_recieved = recv(fd, (void*)buffer, sizeof(buffer), 0);

    // Format
    buffer[bytes_recieved] = '\0';
    std::string res_from_server(buffer);

    if(bytes_recieved <= 0 || res_from_server.find(SERVER_CREATE_LOBBY_GOOD) == std::string::npos){
        std::wcout << "Server error creating a private lobby, try again later..." << std::endl;;;
        return 1;
    }

    // Lobby was created, now we need to display the code on the screen
    std::wstring lobby_code = convertString(res_from_server.substr(CLIENT_INDEX_AFTER_COLON_IN_CREATE_LOBBY_CODE, bytes_recieved - 1));
    
    std::atomic_bool stop_input_thread(false);
    std::thread display_code_and_wait_t(display_code_and_wait, std::ref(lobby_code), std::ref(stop_input_thread));

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
        buffer[bytes_read] = '\0';
        std::string res_from_server(buffer);
        if(bytes_read < 0){
            if(errno != EAGAIN || errno != EWOULDBLOCK){
                // Means a genuine error occurred
                std::wcout << "Error occurred in socket" << std::endl;
                exit(EXIT_FAILURE);
            }
        }else{

            // Got data from the server
            if(res_from_server.compare(MATCH_FOUND) != 0){
                std::wcout << L"\033[2B\033[G" << std::flush; // Move down 2 and to the beggining
                std::wcout << "Error creating/joining lobby for some reason..." << std::endl;
                send(fd, (void*)CLIENT_CLOSE_LOBBY, sizeof(CLIENT_CLOSE_LOBBY), 0);
                return 1;
            }

            //! MATCH FOUND, we need to kill bc it is blocking in wcin
            pthread_kill(display_code_and_wait_t.native_handle(), SIGUSR1);
            display_code_and_wait_t.detach();

            std::wcout << L"\033[2B\033[G" << std::flush; // Move down 2 and to the beggining
            
            //! TEMP
            set_terminal_color(RED);
            std::wcout << "MATCH FOUND = WAITING" << std::endl;
            set_terminal_color(DEFAULT);
            while(true){}
            //game_loop(fd);
            //return 0;
        }

        if(stop_input_thread){
            display_code_and_wait_t.join();
            send(fd, (void*)CLIENT_CLOSE_LOBBY, sizeof(CLIENT_CLOSE_LOBBY), 0);
            return 1;
        }

    }
}

enum CONNECT_TO_SERVER {
    CONNECTING,
    ERROR,
    CONNECTED
};

static std::atomic<CONNECT_TO_SERVER> status(CONNECTING);

void loading_connecting_to_server(){
    
    std::wstring dots = L".";
    int count = 0;

    while (status != CONNECTED || count < 3) {
        
        if (status == ERROR) {
            break;
        }

        std::wcout << L"\rConnecting to server" << dots << std::flush;
        count++;
        dots += L".";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::wcout << std::endl;
}


// 0 no errors
// 1 errors
int connect_to_server(int poll_fd, struct sockaddr_in* poll_addr, int game_fd, struct sockaddr_in* game_addr){
    
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    if (connect(poll_fd, (const sockaddr*)poll_addr, sizeof(*poll_addr)) < 0) {
        std::wcout << L"Connection to server_poll socket failed..." << std::endl;
        status = ERROR;
        return 1;
    }

    // Send bind string
    bind_str = CLIENT_ID_BIND_MSG;
    ONLINE_PLAYER_ID = convertString(generateRandomString(CLIENT_ID_RAND_STRING_LEN));
    bind_str += convertWString(ONLINE_PLAYER_ID);
    send(poll_fd, bind_str.c_str(), bind_str.length(), 0);

    // Receive msg from server
    if(recv(poll_fd, buffer, sizeof(buffer) - 1, 0) <= 0){
        std::wcout << L"Failed to receive from server_poll socket..." << std::endl;
        status = ERROR;
        return 1;
    }


    if(connect(game_fd, (const sockaddr*)game_addr, sizeof(*game_addr)) < 0) {
        std::wcout << L"Connection to game_server socket failed..." << std::endl;
        status = ERROR;
        return 1;
    }

    // Send bind string
    send(game_fd, bind_str.c_str(), bind_str.length(), 0);

    // Receive msg from server
    if(recv(game_fd, buffer, sizeof(buffer) - 1, 0) <= 0){
        std::wcout << L"Failed to receive from game_server socket..." << std::endl;
        status = ERROR;
        return 1;
    }

    status = CONNECTED;

    return 0;

}

int online_game() {

    int client_game_socket_fd = createTCPIPv4Socket();
    int client_poll_socket_fd = createTCPIPv4Socket();
    struct sockaddr_in* server_poll_addr = createIPv4Address("127.0.0.1", 2000);
    struct sockaddr_in* server_game_addr = createIPv4Address("127.0.0.1", 2001);

    std::thread init_connection_animation_t(loading_connecting_to_server);

    int res = connect_to_server(client_poll_socket_fd, server_poll_addr, client_game_socket_fd, server_game_addr);
    
    // Make sure the animation thread joins before continuing execution
    if(init_connection_animation_t.joinable())
        init_connection_animation_t.join();

    if(res == 1)
        return 1;

    // Start polling to the server to keep the connection alive
    std::thread poll_thread(poll_to_server, client_poll_socket_fd);

    // Server was able to bind our sockets together, we are now waiting but without a game_mode
    // On the client we will now show the game modes (Private lobby or random queue)
    // If we choose one we will tell the server

    while(true){
        online_menu();
        int online_opt = get_menu_option();
        if(online_opt == 1){
            // Random queue
            rand_queue_wait(client_game_socket_fd);
        }else if(online_opt == 2){
            // Create private lobby
            create_private_lobby(client_game_socket_fd);
        }else if(online_opt == 3){
            // Join private lobby
            join_private_lobby(client_game_socket_fd);
        }else{
            // Tell server to close Game Socket, this will also clean up the poll socket associated with it
            ONLINE_QUIT = true;
            // Wait for poll thread to join main thread
            if(poll_thread.joinable())
                poll_thread.join();
            close(client_game_socket_fd);
            close(client_poll_socket_fd);
            return 0;
        }
    }


    
}


// //! IMPORTANT

// //! DO NOT ALTER THIS CODE ONLY COPY, THIS TOOK 2 DAYS TO FIGURE OUT

// #include <iostream>
// #include <thread>
// #include <chrono>
// #include <atomic>
// #include <ncurses.h>
// #include <mutex>
// #include <stdio.h>

// std::atomic<bool> stop_timer(false); // Atomic flag to signal the timer thread to stop
// static int current_time = 0;
// static std::string turn_string = "";
// static std::mutex print;
// static char* strings[2] = {"Move piece: ", "To square: "};

// void time_turn_print() {
//     // Use a mutex to ensure thread safety when accessing std::wcout
//     // static std::mutex mtx;
//     // std::lock_guard<std::mutex> lock(mtx);
//     print.lock();
//     //printw("\rTime: %d, Turn: %s", current_time, turn_string.c_str());
//     // std::wcout.flush(); 
//     // int row, col;
//     // getyx(stdscr, row, col); 
//     // move(row, col);

//     std::wcout << "\r                                                              ";
//     std::wcout << "\rTime: " << current_time << " Turn: " << turn_string.c_str() << std::flush;
//     refresh();
//     print.unlock();
// }

// void trackingNcurse() {

//     while (true) {
//         int ch = getch(); 
//         if(ch == 10){
//             std::wcout << "\nSubmit" << std::endl;
//         }
//         else if(ch == 127){
//             if(!turn_string.empty()){
//                 turn_string.pop_back();
//             }
//         }
//         else{
//             turn_string += (char)ch;
//         }
        
//         time_turn_print();
//     }

//     endwin();
// }

// // Function to print the timer
// void timer() {
//     int count = 60; // Initial countdown value
//     while (count > 0 && !stop_timer) {
//         current_time = count;
//         time_turn_print();
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         count--;
//     }
//     std::wcout << std::endl; // Print newline after countdown finishes
// }

// int main() {    
//     initscr();
//     cbreak();
//     noecho();
//     std::thread timer_thread(timer);
//     std::thread ncurses(trackingNcurse);

//     timer_thread.join();

//     return 0;
// }
