#include "./terminal.hpp"
#include <string>
#include <iostream>

void set_terminal_color(enum WRITE_COLOR color) {
    std::wstring col = std::wstring(L"1");// Default color: BOLD WHITE
    switch (color) {
        case RED:
            col = std::wstring(L"31"); // RED
            break;
        case GREEN:
            col = std::wstring(L"32"); // GREEN
            break;
        case BLUE:
            col = std::wstring(L"34"); // BLUE
            break;
        case AQUA:
            col = std::wstring(L"36"); // AQUA
            break;
        case THIN:
            col = std::wstring(L"0"); // THIN WHITE
            break;
        default:
            break;
    }

    std::wstring output = L"\x001b[0;" + col + L"m";
    std::wcout << output;
}

void erase_display(int n) {
    if (n < 0 || n > 3)
        return; // Invalid parameter

    std::wstring output = L"\x001b[" + std::to_wstring(n) + L"J";
    std::wcout << output;
}