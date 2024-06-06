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

void Standard_ChessGame::DEV_MODE_PRESET(){

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
            GameBoard[row][col] = GameSquare(NONE, OPEN, Point(col, row));
            GameBoard[row][col].setFirstMoveMade();
        }
    }

    //        Y  X                                 X, Y
    GameBoard[0][0] = GameSquare(PTWO, ROOK, Point(0, 0));
    GameBoard[0][7] = GameSquare(PTWO, ROOK, Point(7, 0));
    GameBoard[0][3] = GameSquare(PTWO, KING, Point(3, 0));

    GameBoard[7][0] = GameSquare(PONE, ROOK, Point(0, 7));
    GameBoard[7][7] = GameSquare(PONE, ROOK, Point(7, 7));
    GameBoard[7][3] = GameSquare(PONE, KING, Point(3, 7));

    // for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
    //     for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
    //         GameBoard[row][col].print();
    //         std::wcout << std::endl;
    //     }
    // }
}