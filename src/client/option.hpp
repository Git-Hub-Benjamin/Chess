#pragma once
#include "../chess.hpp"

class Options{
public:
    
    enum WRITE_COLOR p1_color = DEFAULT;
    enum WRITE_COLOR p2_color = DEFAULT;

    bool moveHighlighting = false;
    bool boardHistory = false; // UNDO basically
    bool flipBoardOnNewTurn = false;
    
    Options(){}
    Options(const Options& copy): p1_color(copy.p1_color), p2_color(copy.p2_color),
    moveHighlighting(copy.moveHighlighting), boardHistory(copy.boardHistory), flipBoardOnNewTurn(copy.flipBoardOnNewTurn){};

};