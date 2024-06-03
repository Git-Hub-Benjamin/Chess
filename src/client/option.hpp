#pragma once

#include "../terminal-io/colors.hpp"
#include "../Chess/ChessGame/text-piece-art.hpp"

class Options{
public:
    
    enum WRITE_COLOR p1_color = DEFAULT;
    enum WRITE_COLOR p2_color = DEFAULT;

    TEXT_PIECE_ART_COLLECTION_SELECTOR whitePlayerArtSelector = STD_PIECE_ART_P1;
    TEXT_PIECE_ART_COLLECTION_SELECTOR blackPlayerArtSelector = STD_PIECE_ART_P2;

    bool moveHighlighting = false;
    bool boardHistory = false; // UNDO basically
    bool flipBoardOnNewTurn = false;
    
    Options(){}
    Options(const Options& copy): p1_color(copy.p1_color), p2_color(copy.p2_color),
    moveHighlighting(copy.moveHighlighting), boardHistory(copy.boardHistory), flipBoardOnNewTurn(copy.flipBoardOnNewTurn){};

};