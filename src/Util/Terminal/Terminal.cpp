#include "Terminal.hpp"
#include <string>
#include <iostream>

void set_terminal_color(enum WRITE_COLOR color) {
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


    std::wstring output = L"\x001b[0;" + col + L"m";
    std::wcout << output;
}

void clearLine() {
    std::wcout << L"\033[G" << L"\033[K" << std::flush;
}

/*
erase_display(0): Clears from the cursor to the end of the screen.
erase_display(1): Clears from the cursor to the beginning of the screen.
erase_display(2): Clears the entire screen.
erase_display(3): Clears the entire screen and deletes the scrollback buffer.
*/
void erase_display(int n) {
    if (n < 0 || n > 3)
        return; // Invalid parameter

    std::wstring output = L"\x001b[" + std::to_wstring(n) + L"J";
    std::wcout << output;
}