#include "../chess.h"
#include "../server/socket/socketutil.h"
#include <unistd.h>

void online_game(){
    int playerSocketFD = createTCPIPv4Socket();     // server info
    struct sockaddr_in* serverAddr = createIPv4Address("127.0.0.1", 2000);

    int res = connect(playerSocketFD, (const sockaddr*)serverAddr, sizeof(*serverAddr));
    if(res != 0){
        std::wcout << "Connection to server failure." << std::endl;
        exit(EXIT_FAILURE);
    }

    char buffer[1024];

    
    

    while(true){
        
        std::wstring message;
        std::wcin >> message;

        if(message.length() == 1){
            close(playerSocketFD);
            break;
        }
        else
            send(playerSocketFD, message.c_str(), message.length(), 0);
        std::wcout << "Message: " << message.c_str() << ", Length: " << message.length() << std::endl;
        recv(playerSocketFD, buffer, sizeof(buffer), 0);
        buffer[sizeof(buffer) - 1] = '\0'; // Ensure buffer is null-terminated
        std::wcout << "Message from server --> " << buffer << std::endl;
        
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
