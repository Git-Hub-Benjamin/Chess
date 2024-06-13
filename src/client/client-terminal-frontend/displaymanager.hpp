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
    DisplayManager(const int pipe_fd, bool& conditionStop) : pipe_fd(pipe_fd), tellMainWeStopped(conditionStop) {}

    void watchMainThreadWhileInput();

private:
    
    // Members
    bool& tellMainWeStopped; 
    std::wstring inputBuffer; // Current input
    int pipe_fd; // For communication from main thread
    struct pollfd fds[2] = {};
};

