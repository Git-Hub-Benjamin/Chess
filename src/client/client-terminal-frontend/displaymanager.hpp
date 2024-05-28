#include "../../chess.hpp"
#include <atomic>
#include <poll.h>
#include <thread>
#include <mutex>
#include <iostream>
#include <unistd.h>
#include <termios.h> // For terminal I/O functions and termios structure
#include <unistd.h>  // For STDIN_FILENO
#include <cstring>   // For memcpy


#pragma once

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
    DisplayManager(const std::wstring& str, int pipe_fd) : lobbyCode(str), pipe_fd(pipe_fd) {}
    DisplayManager(int pipe_fd) : pipe_fd(pipe_fd) {}

    void start();

private:
    void timer();
    void displayCodeAndWait();
    
    // Members
    std::mutex output;
    std::atomic_bool stop_display; // Only internal, Main thread can cause stop by writing to the end of the pipe
    std::wstring lobbyCode; // Only if needed
    std::wstring inputBuffer; // Current input
    int pipe_fd; // For communication from main thread
    struct pollfd fds[2] = {};
};