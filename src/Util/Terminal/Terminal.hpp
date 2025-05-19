#pragma once
#include "./colors.hpp"
#include "TextPieceArt.hpp"

/*
erase_display(0): Clears from the cursor to the end of the screen.
erase_display(1): Clears from the cursor to the beginning of the screen.
erase_display(2): Clears the entire screen.
erase_display(3): Clears the entire screen and deletes the scrollback buffer.
*/

void erase_display(int n);
void clearLine();
void set_terminal_color(enum WRITE_COLOR color);

