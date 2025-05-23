#include "Terminal.hpp"
#include <iostream> // Already included in .hpp, but good practice to include here too
#include <string>   // Already included in .hpp
#include <vector>   // Might be needed for Windows console info
#include <cstdio>   // For putchar on some systems

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
// Windows (Ensuring UTF-8 is used & Chess Piece Art is displayed)
// 1. Download DejaVu Sans Mono font, Extract, Install
// 2. "Windows Key + R", type "regedit", press Enter
// 3. Navigate to HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
// 4. Right click, New, String Value, Name: 000, Value: DejaVu Sans Mono
// 5. Right click, Properties, Font, DejaVu Sans Mono
// 6. Apply, OK
// 7. Restart command prompt
// 8. "Windows Key + R", type "intl.cpl", press Enter
// 9. Administrative, Set system locale, Toggle "Beta: Use Unicode UTF-8 for worldwide language support"
// 10. Apply, OK, Restart Computer
void WChessPrint(const char* text){
    std::cout << text << std::endl;
}

void WChessPrintFlush(){
    std::cout.flush();
}

void WChessInput(std::string& input){
    std::cin >> input;
}

#else 
// Last testing I did wcout was needed for linux 
void WChessPrint(const wchar_t* text){
     std::wcout << text << std::endl;
}

void WChessPrintFlush(){
    std::wcout.flush();
}

void WChessInput(std::wstring& input){
    std::wcin >> input;
}

#endif

void set_terminal_color(enum WRITE_COLOR color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attributes = 0;

    switch (color) {
        case DEFAULT:
        case WHITE: // Assuming WHITE is your default foreground color
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case BOLD:
            // BOLD is usually handled by a separate escape code on Linux.
            // On Windows, "bold" is often achieved by setting the high intensity flag.
            attributes |= FOREGROUND_INTENSITY;
             // We still need a color, let's default to white with intensity
            attributes |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case BLACK:
            // Black foreground usually has no foreground color bits set
            break;
        case RED:
            attributes = FOREGROUND_RED;
            break;
        case GREEN:
            attributes = FOREGROUND_GREEN;
            break;
        case YELLOW:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN;
            break;
        case BLUE:
            attributes = FOREGROUND_BLUE;
            break;
        case MAGENTA:
            attributes = FOREGROUND_RED | FOREGROUND_BLUE;
            break;
        case AQUA: // Cyan
            attributes = FOREGROUND_GREEN | FOREGROUND_BLUE;
            break;
        case BRIGHT_BLACK: // Dark Gray
            attributes = FOREGROUND_INTENSITY;
            break;
        case BRIGHT_RED:
            attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        case BRIGHT_GREEN:
            attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case BRIGHT_YELLOW:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case BRIGHT_BLUE:
            attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
        case BRIGHT_MAGENTA:
            attributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
        case BRIGHT_AQUA: // Bright Cyan
            attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
        case BRIGHT_WHITE:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
    }

    SetConsoleTextAttribute(hConsole, attributes);

#else // Linux and other Unix-like systems
    std::wstring col = std::wstring(L"0");// Default color: REGULAR THIN WHITE
    switch (color) {
        case BOLD:
            col = std::wstring(L"1"); // BOLD
            break;
        case BLACK:
            col = std::wstring(L"30"); // BLACK
            break;
        case RED:
            col = std::wstring(L"31"); // RED
            break;
        case GREEN:
            col = std::wstring(L"32"); // GREEN
            break;
        case YELLOW:
            col = std::wstring(L"33"); // YELLOW
            break;
        case BLUE:
            col = std::wstring(L"34"); // BLUE
            break;
        case MAGENTA:
            col = std::wstring(L"35"); // MAGENTA
            break;
        case AQUA:
            col = std::wstring(L"36"); // AQUA
            break;
        case WHITE:
            col = std::wstring(L"37"); // WHITE
            break;
        case BRIGHT_BLACK:
            col = std::wstring(L"90"); // BRIGHT BLACK
            break;
        case BRIGHT_RED:
            col = std::wstring(L"91"); // BRIGHT RED
            break;
        case BRIGHT_GREEN:
            col = std::wstring(L"92"); // BRIGHT GREEN
            break;
        case BRIGHT_YELLOW:
            col = std::wstring(L"93"); // BRIGHT YELLOW
            break;
        case BRIGHT_BLUE:
            col = std::wstring(L"94"); // BRIGHT BLUE
            break;
        case BRIGHT_MAGENTA:
            col = std::wstring(L"95"); // BRIGHT MAGENTA
            break;
        case BRIGHT_AQUA:
            col = std::wstring(L"96"); // BRIGHT AQUA
            break;
        case BRIGHT_WHITE:
            col = std::wstring(L"97"); // BRIGHT WHITE
            break;
        case DEFAULT:
        default:
            col = std::wstring(L"0"); // RESET (default)
            break;
    }


    std::wcout << L"\x001b[" << col << L"m"; // Removed "0;" as it's often implicit for reset
#endif
}

void clearLine() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwWritten;
    COORD coordScreen = {0, 0};

    // Get the current cursor position
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return; // Error
    }

    // Set the cursor to the beginning of the current line
    coordScreen.Y = csbi.dwCursorPosition.Y;
    coordScreen.X = 0;
    SetConsoleCursorPosition(hConsole, coordScreen);

    // Fill the line with spaces
    DWORD dwCount = csbi.dwSize.X; // Number of characters in the line
    FillConsoleOutputCharacterW(hConsole, L' ', dwCount, coordScreen, &dwWritten);

    // Reset the attributes of the cleared line to the current attributes
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwCount, coordScreen, &dwWritten);

    // Set the cursor back to the beginning of the line
    SetConsoleCursorPosition(hConsole, coordScreen);

#else // Linux and other Unix-like systems
    std::wcout << L"\033[G" << L"\033[K" << std::flush;
#endif
}

/*
erase_display(0): Clears from the cursor to the end of the screen.
erase_display(1): Clears from the cursor to the beginning of the screen.
erase_display(2): Clears the entire screen.
erase_display(3): Clears the entire screen and deletes the scrollback buffer (Windows equivalent is harder).
*/
void erase_display(int n) {
    if (n < 0 || n > 3)
        return; // Invalid parameter

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwWritten;
    COORD coordScreen = {0, 0};
    DWORD dwCount;

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return; // Error
    }

    switch (n) {
        case 0: // Clears from the cursor to the end of the screen.
            dwCount = csbi.dwSize.X * csbi.dwSize.Y -
                      (csbi.dwCursorPosition.Y * csbi.dwSize.X +
                       csbi.dwCursorPosition.X);
            coordScreen = csbi.dwCursorPosition;
            break;
        case 1: // Clears from the cursor to the beginning of the screen.
            dwCount = csbi.dwCursorPosition.Y * csbi.dwSize.X +
                      csbi.dwCursorPosition.X + 1; // +1 to include the cursor position
            coordScreen = {0, 0};
            break;
        case 2: // Clears the entire screen.
            dwCount = csbi.dwSize.X * csbi.dwSize.Y;
            coordScreen = {0, 0};
            break;
        case 3: // Clears the entire screen and deletes the scrollback buffer.
            // The Windows Console API doesn't have a direct equivalent to
            // clearing the scrollback buffer in the same way as ANSI escape codes.
            // We'll perform a full screen clear (case 2) as the closest equivalent.
            dwCount = csbi.dwSize.X * csbi.dwSize.Y;
            coordScreen = {0, 0};
            // Optionally, you could resize the buffer to a minimal size here
            // but that can be disruptive and is not a direct equivalent.
            break;
    }

    // Fill the affected area with spaces
    FillConsoleOutputCharacterW(hConsole, L' ', dwCount, coordScreen, &dwWritten);

    // Reset the attributes of the cleared area to the current attributes
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwCount, coordScreen, &dwWritten);

    // Set the cursor to the top-left for full screen clears
    if (n == 2 || n == 3) {
        SetConsoleCursorPosition(hConsole, {0, 0});
    }

#else // Linux and other Unix-like systems
    std::wstring output = L"\x001b[" + std::to_wstring(n) + L"J";
    std::wcout << output;
#endif
}