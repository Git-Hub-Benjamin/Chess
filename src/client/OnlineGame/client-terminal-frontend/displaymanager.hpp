#pragma once
#include "Chess/chess.hpp"
#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>

#ifdef __linux__
    #include <poll.h>
    #include <unistd.h>  // For STDIN_FILENO
    #include <unistd.h>
    #include <termios.h> // For terminal I/O functions and termios structure
#endif

#include <cstring>   // For memcpy



//* Also have terminal controller here

class TerminalController {
public:
    TerminalController() {}
    ~TerminalController() {}
private:
#ifdef __linux__
    struct termios old_tio, new_tio;
#endif
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
#ifdef __linux__
    struct pollfd fds[2] = {};
#endif
};

