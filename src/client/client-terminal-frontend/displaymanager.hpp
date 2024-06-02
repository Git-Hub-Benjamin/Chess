#pragma once
#include "../../Chess/chess.hpp"
#include <atomic>
#include <poll.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <termios.h> // For terminal I/O functions and termios structure
#include <unistd.h>  // For STDIN_FILENO
#include <cstring>   // For memcpy



//* Also have terminal controller here

class TerminalController {
public:
    TerminalController() {}
    ~TerminalController() {}
private:
    struct termios old_tio, new_tio;
};

class DisplayManager {
public:
    DisplayManager(const int pipe_fd) : pipe_fd(pipe_fd) { stop_display.store(false); }

    void start_timer_turn_input(); // For in game when you need to have an input by 60 seconds
    void start_input(); // For queue random when you need to wait for the user to do !back
    std::atomic_bool stop_display; 
    // Instance may need to tell main thread we are stopping
    // Main thread can cause stop by writing to the end of the pipe

private:
    void timer();
    void displayCodeAndWait();
    
    // Members
    bool timerRequired = false;
    std::mutex output;
    std::wstring inputBuffer; // Current input
    int pipe_fd; // For communication from main thread
    struct pollfd fds[2] = {};
};