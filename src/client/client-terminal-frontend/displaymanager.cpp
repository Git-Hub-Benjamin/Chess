#include "./displaymanager.hpp"



void DisplayManager::watchMainThreadWhileInput() {
#ifdef __linux__
    struct pollfd fds[2];
    fds[0].fd = fileno(stdin); // File descriptor for std::cin
    fds[0].events = POLLIN;
    fds[1].fd = pipe_fd;       // File descriptor for the read end of the pipe
    fds[1].events = POLLIN;

    while (true) {
        int ret = poll(fds, 2, -1); // Wait indefinitely until an event occurs
        if (ret > 0) {
            if (fds[1].revents & POLLIN) 
                break; // Main thread wants us to stop
            if (fds[0].revents & POLLIN) {
                char ch;
                ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
                if (bytes_read > 0) {
                    if (ch == '\n') {
                        if (inputBuffer == L"!back") {
                            tellMainWeStopped = true;
                            std::wcout << "\033[1A\033[G--> " << std::flush;
                            inputBuffer.clear();
                            break;
                        }
                    } else if (ch == 127) { // Backspace
                        if (!inputBuffer.empty()) {
                            inputBuffer.pop_back();
                            std::wcout << "\033[D \033[D" << std::flush;
                        }
                    } else {
                        std::wcout << ch << std::flush;
                        inputBuffer += ch;
                    }
                }
            }
        }
    }
#endif
}
