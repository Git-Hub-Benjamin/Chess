#include <iostream>
#include <thread>
#include <string>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <cstring>
#include <memory>
#include <atomic>
#include <mutex>
#include <fcntl.h>

#include <iostream>
#include <string>

std::atomic_bool global_stop;
std::mutex mtx;

class TerminalController {
public:
    TerminalController() {
        // Store the original terminal settings
        tcgetattr(STDIN_FILENO, &old_tio);
        memcpy(&new_tio, &old_tio, sizeof(struct termios));
        new_tio.c_lflag &= ~(ICANON | ECHO);
        new_tio.c_cc[VMIN] = 1;
        new_tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    }

    ~TerminalController() {
        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    }

private:
    struct termios old_tio, new_tio;
};

class DisplayManager {
public:
    DisplayManager(const std::wstring& str, int pipe_fd) : lobbyCode(str), pipe_fd(pipe_fd) {}
    DisplayManager(int pipe_fd) : pipe_fd(pipe_fd) {}

    void start() {
        std::thread timer_t(&DisplayManager::timer, this);
        displayCodeAndWait();
        if (timer_t.joinable())
            timer_t.join(); // block until the timer meets
    }

    void startTimer() {
        std::thread timer_t(&DisplayManager::timer, this);
        
    }

private:
    void timer() {
        int count = 60;
        while (count != 0 && !stop_display) {
            // Beggining of the line + Time + current string
            mtx.lock();
            std::wcout << "\033[G" << "Time: " << std::to_wstring(count) << " - " << inputBuffer << std::flush;
            mtx.unlock();
            count--;

            for(int i = 0; i < 4; i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
                if(stop_display)
                    break;
            }
        }
    }

    void displayCodeAndWait() {

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
                    if (buffer[0] == '1')
                        break;

                    // Checking if main thread wants this one to stop
                }
                if (fds[0].revents & POLLIN) {
                    char ch;
                    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
                    if (bytes_read > 0) {
                        mtx.lock();
                        if (ch == '\n') {
                            if(inputBuffer == L"!back")
                                break; // Handle out
                        } else if (ch == 127) { // Backspace
                            if (!inputBuffer.empty()) {
                                inputBuffer.pop_back();
                                std::wcout << "\033[D \033[D" << std::flush;
                            }
                        } else {
                            std::wcout << ch << std::flush;
                            inputBuffer += ch;
                        }
                        mtx.unlock();
                    }
                }
            }
        }

        stop_display.store(true);
        
        std::wcout << "\033[GStopping display_code_and_wait...\n" << std::flush;
    }

    // Members
    std::atomic_bool stop_display; // Only internal, Main thread can cause stop by writing to the end of the pipe
    std::wstring lobbyCode; // Only if needed
    std::wstring inputBuffer; // Current input
    int pipe_fd; // For communication from main thread
    struct pollfd fds[2] = {};
};

int main(){
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }

    {
        DisplayManager displayCodeAndWait(pipe_fd[0]);
        TerminalController terminalcontroller;
        std::atomic_bool& stop_from_input_t = displayCodeAndWait.stop_display;
        display_rand_queue_menu();
        std::thread lobbyInput_t(&DisplayManager::start_input, &displayCodeAndWait);

        while (true) {

            std::string oppBindStr;
            enum Owner playerNumForMatch;
            char buffer[ONLINE_BUFFER_SIZE] = {0};

            int bytes_read = recv(fd, (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::string res_from_server(buffer);

                if (check_for_match_found(res_from_server, oppBindStr, playerNumForMatch)) {
                    write(pipe_fd[1], "1", 1);
                    if (lobbyInput_t.joinable())
                        lobbyInput_t.join();

                    std::wcout << L"\033[2B\033[G" << std::flush;
                    game_loop(fd, playerNumForMatch, oppBindStr);
                    return 0;
                } else {
                    std::wcout << L"\033[2B\033[G" << std::flush;
                    std::wcout << "Error joining lobby for some reason..." << std::endl;
                    return 1;
                }
            } else if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                std::wcout << "Error occurred in socket" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (stop_from_input_t) {
                if (lobbyInput_t.joinable())
                    lobbyInput_t.join();

                std::wcout << "Stop from input thread was set to true" << std::endl;

                send(fd, (void*)CLIENT_LEAVE_RANDOM_QUEUE, sizeof(CLIENT_LEAVE_RANDOM_QUEUE), 0);
                return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ensure some delay to prevent busy waiting
        }
    }
}


// int main() {
//     std::wstring lobby_code = L"LobbyCode";

//     // Create a pipe for signaling
//     int pipe_fd[2];
//     if (pipe(pipe_fd) == -1) {
//         std::cerr << "Failed to create pipe" << std::endl;
//         return 1;
//     }

    
//     {
//         DisplayManager display_timer(pipe_fd[0]);
//         TerminalController terminalController; // Manage terminal settings
//         std::thread display_t(&DisplayManager::start, &display_timer);
        
//         // Simulate some work
//         std::this_thread::sleep_for(std::chrono::seconds(5));

//         // Signal the thread to stop
//         write(pipe_fd[1], "1", 2); // Send termination signal to the pipe

//         // Wait for the thread to complete execution
//         if (display_t.joinable()) {
//             display_t.join();
//         }
//     }


//     return 0;
// }


// int main(int argc, char **argv)
// {
//     struct termios attr;
//     tcgetattr(STDIN_FILENO, &attr);
//     attr.c_lflag &= ~(ICANON | ECHO);
//     tcsetattr(STDIN_FILENO, TCSANOW, &attr);
//     uint8_t buf[20];
//     ssize_t bytes;
//     while ((bytes = read(STDIN_FILENO, buf, 20)) > 0) {
//         for (size_t i = 0; i < bytes; i++) {
//             printf("%zd: %hhu\n", i, buf[i]);
//         }
//     }
//     return 0;
// }








// #include <iostream>
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_ttf.h>
// #include <string>
// #include <chrono>
// #include <thread>
// #include <atomic>

// // Function to render text
// SDL_Texture* renderText(const std::string& text, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
//     SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
//     if (surface == nullptr) {
//         std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << std::endl;
//         return nullptr;
//     }
//     SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
//     SDL_FreeSurface(surface);
//     return texture;
// }

// // Timer function
// void timerThread(std::atomic<bool>& stopTimer, int& duration) {
//     auto startTime = std::chrono::steady_clock::now();
//     while (duration > 0 && !stopTimer) {
//         auto currentTime = std::chrono::steady_clock::now();
//         auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
//         duration = 60 - static_cast<int>(elapsedTime);
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//     }
// }

// int main(int argc, char* argv[]) {
//     // Initialize SDL
//     if (SDL_Init(SDL_INIT_VIDEO) != 0) {
//         std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
//         return 1;
//     }

//     // Initialize SDL_ttf
//     if (TTF_Init() == -1) {
//         std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
//         SDL_Quit();
//         return 1;
//     }

//     // Create a window
//     SDL_Window* window = SDL_CreateWindow("SDL Timer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
//     if (window == nullptr) {
//         std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
//         TTF_Quit();
//         SDL_Quit();
//         return 1;
//     }

//     // Create a renderer
//     SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
//     if (renderer == nullptr) {
//         std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
//         SDL_DestroyWindow(window);
//         TTF_Quit();
//         SDL_Quit();
//         return 1;
//     }

//     // Load font
//     TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/msttcorefonts/arial.ttf", 24);
//     if (font == nullptr) {
//         std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
//         SDL_DestroyRenderer(renderer);
//         SDL_DestroyWindow(window);
//         TTF_Quit();
//         SDL_Quit();
//         return 1;
//     }

//     // Set render color to black
//     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

//     // Create a flag to control the timer
//     std::atomic<bool> stopTimer(false);
//     int duration = 60;

//     // Start the timer thread
//     std::thread timer(timerThread, std::ref(stopTimer), std::ref(duration));

//     // Input box parameters
//     std::string userInput = "";
//     SDL_Color textColor = {0, 0, 0, 255}; // Black color
//     int inputBoxWidth = 300;
//     int inputBoxHeight = 50;
//     int inputBoxX = (640 - inputBoxWidth) / 2;
//     int inputBoxY = (480 - inputBoxHeight) / 2;

//     // Main loop
//     bool quit = false;
//     SDL_Event event;
//     while (!quit) {
//         // Handle events
//         while (SDL_PollEvent(&event)) {
//             if (event.type == SDL_QUIT) {
//                 quit = true;
//             } else if (event.type == SDL_TEXTINPUT) {
//                 // Handle text input
//                 userInput += event.text.text;
//             } else if (event.type == SDL_KEYDOWN) {
//                 // Handle backspace
//                 if (event.key.keysym.sym == SDLK_BACKSPACE && !userInput.empty()) {
//                     userInput.pop_back();
//                 }
//             }
//         }

//         // Clear the screen
//         SDL_RenderClear(renderer);

//         // Render input box outline
//         SDL_Rect inputBoxRect = {inputBoxX, inputBoxY, inputBoxWidth, inputBoxHeight};
//         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
//         SDL_RenderDrawRect(renderer, &inputBoxRect);

//         // Render user input text
//         // Render user input text
//         if (!userInput.empty()) {
//             SDL_Texture* userInputTexture = renderText(userInput, font, textColor, renderer);
//             if (userInputTexture != nullptr) {
//                 int textWidth, textHeight;
//                 SDL_QueryTexture(userInputTexture, nullptr, nullptr, &textWidth, &textHeight);
//                 SDL_Rect textRect = {inputBoxX + 5, inputBoxY + (inputBoxHeight - textHeight) / 2, textWidth, textHeight};
//                 SDL_RenderCopy(renderer, userInputTexture, nullptr, &textRect);
//                 SDL_DestroyTexture(userInputTexture);
//             }
//         }

//         // Render timer if duration is positive
//         if (duration > 0) {
//             std::string timerText = "Time left: " + std::to_string(duration) + "s";
//             SDL_Texture* timerTexture = renderText(timerText, font, textColor, renderer);
//             if (timerTexture != nullptr) {
//                 int textWidth, textHeight;
//                 SDL_QueryTexture(timerTexture, nullptr, nullptr, &textWidth, &textHeight);
//                 SDL_Rect timerRect = {10, 10, textWidth, textHeight}; // Top left corner
//                 SDL_RenderCopy(renderer, timerTexture, nullptr, &timerRect);
//                 SDL_DestroyTexture(timerTexture);
//             }
//         }

//         // render button
//         // Render button
//         SDL_Rect buttonRect = {640 - 110, 480 - 60, 100, 50};
//         SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue color
//         SDL_RenderFillRect(renderer, &buttonRect);

//         // Render button text
//         SDL_Color buttonTextColor = {255, 255, 255, 255}; // White color
//         SDL_Texture* buttonTextTexture = renderText("Click", font, buttonTextColor, renderer);
//         if (buttonTextTexture != nullptr) {
//             int textWidth, textHeight;
//             SDL_QueryTexture(buttonTextTexture, nullptr, nullptr, &textWidth, &textHeight);
//             SDL_Rect textRect = {640 - 110 + (100 - textWidth) / 2, 480 - 60 + (50 - textHeight) / 2, textWidth, textHeight};
//             SDL_RenderCopy(renderer, buttonTextTexture, nullptr, &textRect);
//             SDL_DestroyTexture(buttonTextTexture);
//         }

//         // Check if the button is clicked
//         int mouseX, mouseY;
//         Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
//         bool isButtonClicked = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && mouseX >= buttonRect.x && mouseX < buttonRect.x + buttonRect.w && mouseY >= buttonRect.y && mouseY < buttonRect.y + buttonRect.h;

//         // Perform action when the button is clicked
//         if (isButtonClicked) {
//             // Action to perform when the button is clicked (e.g., display a message)
//             std::wcout << "Button clicked!" << std::endl;
//         }



//         // Present the renderer
//         SDL_RenderPresent(renderer);

//         // Delay to reduce CPU usage
//         SDL_Delay(10);
//     }

//     // Stop the timer thread
//     stopTimer = true;
//     if (timer.joinable()) {
//         timer.join();
//     }

//     // Clean up
//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyWindow(window);
//     TTF_CloseFont(font);
//     TTF_Quit();
//     SDL_Quit();
//     return 0;
// }
