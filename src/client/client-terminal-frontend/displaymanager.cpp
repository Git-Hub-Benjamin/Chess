#include "./displaymanager.hpp"

void DisplayManager::start_input(){
    displayCodeAndWait();
}

void DisplayManager::timer() {
    int count = 60;
    while (count != 0 && !stop_display) {
        // Beggining of the line + Time + current string
        std::lock_guard<std::mutex> lock(output);
        std::wcout << "\033[G" << "Time: " << std::to_wstring(count) << " - " << inputBuffer << std::flush;
        count--;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void DisplayManager::displayCodeAndWait() {

    struct pollfd fds[2];
    fds[0].fd = fileno(stdin); // File descriptor for std::cin
    fds[0].events = POLLIN;
    fds[1].fd = pipe_fd;       // File descriptor for the read end of the pipe
    fds[1].events = POLLIN;

    while (!stop_display) {
        int ret = poll(fds, 2, -1); // Wait indefinitely until an event occurs
        if (ret > 0) {
            if (fds[1].revents & POLLIN) {
                // Pipe has data to read
                char buffer[32];
                int byte_read = read(fds[1].fd, (void *)buffer, sizeof(buffer));
                
                std::wcout << "Data to be read from the pipe" << std::endl;
                if (buffer[0] == '1')
                    break;
            }
            if (fds[0].revents & POLLIN) {
                char ch;
                ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
                if (bytes_read > 0) {
                    std::lock_guard<std::mutex> lock(output); 
                    if (ch == '\n') {

                        if(!timerRequired){
                            if (inputBuffer == L"!back")
                                break;
                            std::wcout << "\033[1A\033[G--> " << std::flush;
                            inputBuffer.clear();
                        }else {
                            // For timer
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

    stop_display.store(true);
}
