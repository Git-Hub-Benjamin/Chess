#include <iostream>
#include <thread>
#include <csignal>


// Signal handler to interrupt std::cin
void signal_handler(int signum) {}

// Function that runs in the thread
void thread_func() {
    std::string input;
    // Register the signal handler for SIGUSR1
    signal(SIGUSR1, signal_handler);

    while (true) {
        std::cout << "Enter input: ";
        // Try to read input
        std::cin >> input;
        std::cout << "You entered: " << input << std::endl;
    }

    std::cout << "Thread exiting gracefully." << std::endl;
}

int main() {
    std::thread t(thread_func);

    // Give the thread time to start and block on std::cin
    
    sleep(3);

    std::cout << "\nMatch found..." << std::endl;

    // Send the SIGUSR1 signal to the thread to interrupt std::cin
    pthread_kill(t.native_handle(), SIGUSR1);

    std::wstring in;
    std::wcin >> in;

    return 0;
}





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
