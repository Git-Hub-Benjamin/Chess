#include "./terminal.hpp"

void set_terminal_color(enum WRITE_COLOR color) {
    std::wstring col = std::wstring(L"37");// Default color: WHITE
    switch (color) {
        case RED:
            col = std::wstring(L"31"); // RED
            break;
        case GREEN:
            col = std::wstring(L"32"); // GREEN
            break;
        case AQUA:
            col = std::wstring(L"36"); // AQUA
            break;
        case BOLD:
            col = std::wstring(L"1"); // AQUA
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