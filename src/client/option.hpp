#pragma once

#include "../terminal-io/colors.hpp"
#include "../Chess/ChessGame/text-piece-art.hpp"

extern bool SETTING_CHANGE_AFFECTS_CONFIG_FILE;
class Options{
public:
    
    enum WRITE_COLOR p1_color = DEFAULT;
    enum WRITE_COLOR p2_color = DEFAULT;

    TEXT_PIECE_ART_COLLECTION_SELECTOR whitePlayerArtSelector = STD_PIECE_ART_P1;
    TEXT_PIECE_ART_COLLECTION_SELECTOR blackPlayerArtSelector = STD_PIECE_ART_P2;

    bool dynamicMoveHighlighting = false;
    bool moveHighlighting = false;
    bool boardHistory = false; // UNDO / REDO basically
    bool flipBoardOnNewTurn = false;
    bool clearScreenOnBoardPrint = false;
    
    Options(){}
    Options(const Options& copy): p1_color(copy.p1_color), p2_color(copy.p2_color),
    moveHighlighting(copy.moveHighlighting), clearScreenOnBoardPrint(copy.clearScreenOnBoardPrint), dynamicMoveHighlighting(copy.dynamicMoveHighlighting), boardHistory(copy.boardHistory), flipBoardOnNewTurn(copy.flipBoardOnNewTurn),
    whitePlayerArtSelector(copy.whitePlayerArtSelector), blackPlayerArtSelector(copy.blackPlayerArtSelector) {};

    void print() const {
        std::wcout << "p1_color: " << writeColorToString(p1_color) << std::endl;
        std::wcout << "p2_color: " << writeColorToString(p2_color) << std::endl;
        std::wcout << "whitePlayerArtSelector: " << artSelectorToString(whitePlayerArtSelector) << std::endl;
        std::wcout << "blackPlayerArtSelector: " << artSelectorToString(blackPlayerArtSelector) << std::endl;
        std::wcout << "moveHighlighting: " << (moveHighlighting ? "true" : "false") << std::endl;
        std::wcout << "boardHistory: " << (boardHistory ? "true" : "false") << std::endl;
        std::wcout << "flipBoardOnNewTurn: " << (flipBoardOnNewTurn ? "true" : "false") << std::endl;
    }

private:
    const char* writeColorToString(WRITE_COLOR color) const {
        switch (color) {
            case DEFAULT: return "DEFAULT";
            case RED: return "RED";
            case GREEN: return "GREEN";
            case BLUE: return "BLUE";
            default: return "UNKNOWN";
        }
    }

    const char* artSelectorToString(TEXT_PIECE_ART_COLLECTION_SELECTOR selector) const {
        switch (selector) {
            case STD_PIECE_ART_P1: return "STD_PIECE_ART_P1";
            case STD_PIECE_ART_P2: return "STD_PIECE_ART_P2";
            case STD_PIECE_CHAR_P1: return "STD_PIECE_CHAR_P1";
            case STD_PIECE_CHAR_P2: return "STD_PIECE_CHAR_P2";
            default: return "UNKNOWN";
        }
    }
};