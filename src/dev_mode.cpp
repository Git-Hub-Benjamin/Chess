#include "./Chess/chess.hpp"

int dev_menu_option(){
	while(true){
		std::wstring str;
		std::wcin >> str;
		if(str[0] == L'1')
			return 1;
		if(str[0] == L'2')
			return 2;
	}
	return -1;
}

void StandardLocalChessGame::DEV_MODE_PRESET(){

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
            GameBoard[row][col] = GameSquare(NONE, OPEN, Point(col, row));
            GameBoard[row][col].setFirstMoveMade();
        }
    }

    //        Y  X                                 X, Y
    GameBoard[2][0] = GameSquare(PTWO, ROOK, Point(0, 2));
    GameBoard[0][2] = GameSquare(PTWO, PAWN, Point(2, 0));
    GameBoard[0][4] = GameSquare(PTWO, PAWN, Point(4, 0));
    GameBoard[1][4] = GameSquare(PTWO, PAWN, Point(4, 1));
    GameBoard[0][7] = GameSquare(PTWO, ROOK, Point(7, 0));
    GameBoard[1][3] = GameSquare(PTWO, KING, Point(3, 1));

    GameBoard[1][0] = GameSquare(PONE, ROOK, Point(0, 1));
    GameBoard[6][3] = GameSquare(PONE, ROOK, Point(3, 6));
    GameBoard[7][7] = GameSquare(PONE, ROOK, Point(7, 7));
    GameBoard[7][3] = GameSquare(PONE, KING, Point(3, 7));

    currentTurn = PlayerTwo;
    blackPlayerKing = &GameBoard[7][3];

    // for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
    //     for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
    //         GameBoard[row][col].print();
    //         std::wcout << std::endl;
    //     }
    // }
}