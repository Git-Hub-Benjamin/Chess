#pragma once

#include <iostream>
#include <string>
#include <vector> // Might be needed for Windows console info

#ifdef _WIN32
#include <windows.h>
#endif

// Assuming your enum WRITE_COLOR is defined elsewhere, include its header
// #include "your_color_enum_header.hpp"

enum WRITE_COLOR {
    DEFAULT,
    BOLD,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    AQUA,
    WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_AQUA,
    BRIGHT_WHITE
};


void setTerminalColor(enum WRITE_COLOR color);
void clearLine();
void eraseDisplay();

void WChessPrintFlush();
void WChessPrint(const char* text);
void WChessInput(std::string& input);
void WChessPrint(const wchar_t* text);
void WChessInput(std::wstring& input);

/*
erase_display(0): Clears from the cursor to the end of the screen.
erase_display(1): Clears from the cursor to the beginning of the screen.
erase_display(2): Clears the entire screen.
erase_display(3): Clears the entire screen and deletes the scrollback buffer.
*/

