#include "../socket/socketutil.h"
#include "./server-terminal-communication/servercommands.hpp"
#include "../Chess/chess.hpp"
#include "server.hpp"


static int server_terminal_communication_fd = -1;


void signalHandler(int signal) {
    void *array[10];
    size_t size;

    // Get the array of void*'s for the backtrace.
    size = backtrace(array, 10);

    // Print out the backtrace to stderr.
    std::cerr << "Error: signal " << signal << ":" << std::endl;
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main(){

    std::locale::global(std::locale("en_US.UTF-8"));

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);  // Catch segmentation faults
    std::signal(SIGABRT, signalHandler); // Handle abort signal
    std::signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE signals

    #ifdef SERVER_TERMINAL
const char* fifo_path = "/tmp/serverchessfifo";
    server_terminal_communication_fd = open(fifo_path, O_RDONLY | O_NONBLOCK);

    if (server_terminal_communication_fd == -1) {
        perror("open");
        
        char user_input;
        std::wcout << L"FIFO does not exist. Do you want to create it? (y/n): ";
        std::cin >> user_input;

        if (user_input == 'y' || user_input == 'Y') {
            if (mkfifo(fifo_path, 0777) == -1) {
                perror("mkfifo");
                return 1;
            } else {
                std::wcout << L"FIFO created successfully. Waiting until writer is available..." << std::endl;
                server_terminal_communication_fd = open(fifo_path, O_RDONLY | O_NONBLOCK);
                if (server_terminal_communication_fd == -1) {
                    perror("open");
                    return 1;
                }
            }
        } else {
            return 1;
        }
    }

    // Set the file descriptor for non-blocking mode
    int flags = fcntl(server_terminal_communication_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        return 1;
    }
    if (fcntl(server_terminal_communication_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL)");
        return 1;
    }

    std::wcout << "Reading end of server-fifo-terminal ready for reading..." << std::endl;
    #endif

    // init server
    int poll_socket_fd = createTCPIPv4Socket();
    struct sockaddr_in* poll_socket_addr = createIPv4Address("", 2000);

    int client_game_sock_fd = createTCPIPv4Socket();
    struct sockaddr_in* client_game_sock_addr = createIPv4Address("", 2001);

    if (fcntl(poll_socket_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl failed");
        exit(EXIT_FAILURE);
    }

    if (fcntl(client_game_sock_fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("fcntl failed");
        exit(EXIT_FAILURE);
    }

    if(bind(poll_socket_fd, (struct sockaddr*)poll_socket_addr, sizeof(*poll_socket_addr)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    if(listen(poll_socket_fd, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        close(poll_socket_fd);
        exit(EXIT_FAILURE);
    }

    if(bind(client_game_sock_fd, (struct sockaddr*)client_game_sock_addr, sizeof(*client_game_sock_addr)) != 0){
        std::wcout << "Error binding socket." << std::endl;
        close(poll_socket_fd);
        exit(EXIT_FAILURE);
    }

    if(listen(client_game_sock_fd, MAX_CLIENT) != 0){
        std::wcout << "Error listening." << std::endl;
        close(poll_socket_fd);
        close(client_game_sock_fd);
        exit(EXIT_FAILURE);
    }

    WChessServer Server(poll_socket_fd, client_game_sock_fd, server_terminal_communication_fd);

    std::wcout << "Server listening for clients..." << std::endl;
    Server.startServer();

    // Server is setup now...

    return 0;
}


