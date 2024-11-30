#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <iostream>
#include "../../terminal-io/colors.hpp"
#include "../../terminal-io/terminal.hpp"

void toLowercase(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
}

void printCommands() {
    set_terminal_color(DEFAULT);
    std::printf("Command                         ");
    set_terminal_color(AQUA);
    std::printf("Description\n");
    
    set_terminal_color(DEFAULT);
    std::printf("---------------------------------");
    set_terminal_color(AQUA);
    std::printf(" -------------------------------------------------\n");

    set_terminal_color(DEFAULT);
    std::printf("kill                            ");
    set_terminal_color(AQUA);
    std::printf("Terminate the server.\n");

    set_terminal_color(DEFAULT);
    std::printf("all-clients (alias: aclients)   ");
    set_terminal_color(AQUA);
    std::printf("Displays all clients.\n");

    set_terminal_color(DEFAULT);
    std::printf("wait-clients (alias: wclients)  ");
    set_terminal_color(AQUA);
    std::printf("Displays all clients in waiting state.\n");

    set_terminal_color(DEFAULT);
    std::printf("play-clients (alias: pclients)  ");
    set_terminal_color(AQUA);
    std::printf("Displays all clients in playing state.\n");

    set_terminal_color(DEFAULT);
    std::printf("game-clients (alias: lclients)  ");
    set_terminal_color(AQUA);
    std::printf("Displays active games and the players in them.\n");

    set_terminal_color(DEFAULT);
    std::printf("private-lobby-clients (alias: privclients) ");
    set_terminal_color(AQUA);
    std::printf("Displays created private lobbies and their clients.\n");

    set_terminal_color(DEFAULT);
    std::printf("kill-lobbies (alias: kthlobby)  ");
    set_terminal_color(AQUA);
    std::printf("Kills all lobbies and terminates the associated thread.\n");

    set_terminal_color(DEFAULT);
    std::printf("poll-clients (alias: plclients) ");
    set_terminal_color(AQUA);
    std::printf("Displays polling clients and their file descriptors.\n");

    set_terminal_color(DEFAULT);
}

static int static_fd;

int main() {
    char input[128];

    const char* fifo_path = "/tmp/serverchessfifo";

    printf("Waiting until server is open & ready to read fifo cmds...\n");
    fflush(stdout);  // Ensure this message is displayed immediately
    static_fd = open(fifo_path, O_WRONLY);

    if (static_fd == -1) {
        perror("open");

        char user_input;
        printf("FIFO does not exist. Do you want to create it? (y/n): ");
        fflush(stdout);  // Ensure this message is displayed immediately

        std::cin >> user_input;

        if (user_input == 'y' || user_input == 'Y') {
            if (mkfifo(fifo_path, 0777) == -1) {
                perror("mkfifo");
                return 1;
            } else {
                printf("FIFO created successfully. Waiting until server is open & ready to read fifo cmds...\n");
                fflush(stdout);  // Ensure this message is displayed immediately
                static_fd = open(fifo_path, O_WRONLY);
                if (static_fd == -1) {
                    perror("open");
                    return 1;
                }
            }
        } else {
            return 1;
        }
    }

    std::string last_cmd;
    printCommands();
    printf("Enter text to send to the reader (Ctrl+D to quit):\nCommand: ");
    fflush(stdout);  // Ensure this message is displayed immediately

    while (fgets(input, sizeof(input), stdin)) {
        // Remove the newline character if it exists
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Check if using old command
        std::string cmd(input);

        if (cmd == "\033[A")  // This won't work for detecting arrow keys, just for illustration
            cmd = last_cmd;
        else {
            if (cmd.length() == 0)
                continue;

            // To lower case
            toLowercase(cmd);        
            last_cmd = cmd;
        }

        ssize_t bytesWritten = write(static_fd, cmd.c_str(), cmd.length());
        if (bytesWritten == -1) {
            perror("write");
            close(static_fd);
            return 1;
        }

        printf("Command: ");
        fflush(stdout);  // Ensure this message is displayed immediately
    }

    close(static_fd);
    return 0;
}
