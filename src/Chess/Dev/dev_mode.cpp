#include <string>
#include "../StandardLocalChessGame.hpp"

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
#ifdef LEGACY_ARRAY_GAMEBOARD
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
            GameBoard[row][col] = GameSquare(ChessTypes::Owner::None, ChessTypes::GamePiece::None, Point(col, row));
            GameBoard[row][col].setFirstMoveMade();
        }
    }

    //        Y  X                                 X, Y
    GameBoard[2][0] = GameSquare(ChessTypes::Owner::PlayerTwo, ChessTypes::GamePiece::Rook, Point(0, 2));
    GameBoard[0][2] = GameSquare(ChessTypes::Owner::PlayerTwo, ChessTypes::GamePiece::Pawn, Point(2, 0));
    GameBoard[0][4] = GameSquare(ChessTypes::Owner::PlayerTwo, ChessTypes::GamePiece::Pawn, Point(4, 0));
    GameBoard[1][4] = GameSquare(ChessTypes::Owner::PlayerTwo, ChessTypes::GamePiece::Pawn, Point(4, 1));
    GameBoard[0][7] = GameSquare(ChessTypes::Owner::PlayerTwo, ChessTypes::GamePiece::Rook, Point(7, 0));
    GameBoard[1][3] = GameSquare(ChessTypes::Owner::PlayerTwo, ChessTypes::GamePiece::King, Point(3, 1));

    GameBoard[1][0] = GameSquare(ChessTypes::Owner::PlayerOne, ChessTypes::GamePiece::Rook, Point(0, 1));
    GameBoard[6][3] = GameSquare(ChessTypes::Owner::PlayerOne, ChessTypes::GamePiece::Rook, Point(3, 6));
    GameBoard[7][7] = GameSquare(ChessTypes::Owner::PlayerOne, ChessTypes::GamePiece::Rook, Point(7, 7));
    GameBoard[7][3] = GameSquare(ChessTypes::Owner::PlayerOne, ChessTypes::GamePiece::King, Point(3, 7));

    currentTurn = ChessTypes::Player::PlayerTwo;
    blackPlayerKing = &GameBoard[7][3];

    // for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
    //     for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {
    //         GameBoard[row][col].print();
    //         std::wcout << std::endl;
    //     }
    // }
#endif
}