#pragma once

#include "../../Util/Terminal/Terminal.hpp"
#include "../../Util/Terminal/TextPieceArt.hpp"
#include <iostream>


enum GET_MENU_OPTION {
    DEV = -1,
    ONE = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    BACK,
    INVALID
};

#define THR_MIN 60 * 3
#define FIVE_MIN 60 * 3
#define FIF_TEEN_MIN 60 * 3

class Options {
    public:
    enum WRITE_COLOR p1_color = DEFAULT;
    enum WRITE_COLOR p2_color = DEFAULT;

    enum WRITE_COLOR movingPiece_color = GREEN;
    enum WRITE_COLOR possibleMove_color = BRIGHT_RED;
    enum WRITE_COLOR movingToPiece_color = BRIGHT_GREEN;

    TEXT_PIECE_ART_COLLECTION_SELECTOR whitePlayerArtSelector = STD_PIECE_ART_P1;
    TEXT_PIECE_ART_COLLECTION_SELECTOR blackPlayerArtSelector = STD_PIECE_ART_P2;

    // This setting updates and prints gameboard everytime a character is typed so its very complex and computationally expensive, also hell to implement in windows terminal
    bool dynamicMoveHighlighting = false;
    // This setting calls LprintBoardWithMoves instead of LprintBoard to show possible moves after choose fromPiece
    bool moveHighlighting = false;
    // This setting allows for undo and redo, not implemented 
    bool boardHistory = false; // UNDO / REDO basically
    // This will flip the board on new turn, half implemented
    bool flipBoardOnNewTurn = false;
    // Clears the screen every time a new (options screen, game screen, or board) is printed
    bool clearScreenOnPrint = false;

    Options() {}
    Options(const Options &copy) : p1_color(copy.p1_color), p2_color(copy.p2_color),
                                   moveHighlighting(copy.moveHighlighting), clearScreenOnPrint(copy.clearScreenOnPrint), dynamicMoveHighlighting(copy.dynamicMoveHighlighting), boardHistory(copy.boardHistory), flipBoardOnNewTurn(copy.flipBoardOnNewTurn),
                                   whitePlayerArtSelector(copy.whitePlayerArtSelector), blackPlayerArtSelector(copy.blackPlayerArtSelector) {};

    void print() const
    {
        std::wcout << "p1_color: " << writeColorToString(p1_color) << std::endl;
        std::wcout << "p2_color: " << writeColorToString(p2_color) << std::endl;
        std::wcout << "whitePlayerArtSelector: " << artSelectorToString(whitePlayerArtSelector) << std::endl;
        std::wcout << "blackPlayerArtSelector: " << artSelectorToString(blackPlayerArtSelector) << std::endl;
        std::wcout << "moveHighlighting: " << (moveHighlighting ? "true" : "false") << std::endl;
        std::wcout << "boardHistory: " << (boardHistory ? "true" : "false") << std::endl;
        std::wcout << "flipBoardOnNewTurn: " << (flipBoardOnNewTurn ? "true" : "false") << std::endl;
    }

private:
    const char *writeColorToString(WRITE_COLOR color) const
    {
        switch (color)
        {
        case DEFAULT:
            return "DEFAULT";
        case RED:
            return "RED";
        case GREEN:
            return "GREEN";
        case BLUE:
            return "BLUE";
        default:
            return "UNKNOWN";
        }
    }

    const char *artSelectorToString(TEXT_PIECE_ART_COLLECTION_SELECTOR selector) const
    {
        switch (selector)
        {
        case STD_PIECE_ART_P1:
            return "STD_PIECE_ART_P1";
        case STD_PIECE_ART_P2:
            return "STD_PIECE_ART_P2";
        case STD_PIECE_CHAR_P1:
            return "STD_PIECE_CHAR_P1";
        case STD_PIECE_CHAR_P2:
            return "STD_PIECE_CHAR_P2";
        default:
            return "UNKNOWN";
        }
    }
};

// false (BACK BUTTON)
// true (good)
bool change_player_color_option();
void overwrite_option_file();
bool erase_config_file();
enum GET_MENU_OPTION get_menu_option();
extern Options global_player_option;
extern std::string CONFIG_PATH;
extern bool SETTING_CHANGE_AFFECTS_CONFIG_FILE;