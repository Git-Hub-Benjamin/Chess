#include <iostream>
#include "chess.h"

// Game.GameBoard[moveToSquare->pos.y][moveToSquare->pos.x].piece == movePiece->piece;
// Game.GameBoard[moveToSquare->pos.y][moveToSquare->pos.x].ownership == movePiece->ownership;
// Game.GameBoard[from.pos.y][from.pos.x].ownership == NONE;
// Game.GameBoard[from.pos.y][from.pos.x].piece == OPEN;

using namespace std;
int testing[8][8] = {0};

void printboard(){
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            cout << "| " << testing[i][j] << " ";
        }
        cout << "|" << endl;
    }
}
int main() {
    ChessGame Game;
    GameSqaure* moveToSquare = new GameSqaure;
    GameSqaure* movePiece = new GameSqaure;
    movePiece->piece = 
    moveToSquare->pos.y = 1;
    moveToSquare->pos.y = 1;


    Game.GameBoard[moveToSquare->pos.y][moveToSquare->pos.x].piece == movePiece->piece;

    testing[0][1] = 1;
    printboard(); 
}
