#include "./chess.hpp"

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

void ChessGame::DEV_MODE_PRESET(){
    // std::wcout << "1. Rook Preset\n2. Bishop Preset" << std::endl;
    // switch(dev_menu_option()){
    //     case 1: // Rook, King // GameBoard[row][col].pos = {col, row};
            
    //         break;
    //     case 2:
    //         break;
    // }

    // PLAYER ONE

    this->GameBoard[7][0].ownership = PONE;
    this->GameBoard[7][0].piece = ROOK;
    
    this->GameBoard[7][7].ownership = PONE;
    this->GameBoard[7][7].piece = ROOK;
    
    this->GameBoard[7][4].ownership = PONE;
    this->GameBoard[7][4].piece = KING;
    this->KingPositions[0] = &this->GameBoard[7][4];

    this->GameBoard[7][3].ownership = PONE;
    this->GameBoard[7][3].piece = QUEEN;

    this->GameBoard[7][5].ownership = PONE;
    this->GameBoard[7][5].piece = BISHOP;

    // PLAYER TWO

    this->GameBoard[0][0].ownership = PTWO;
    this->GameBoard[0][0].piece = ROOK;
    
    this->GameBoard[0][7].ownership = PTWO;
    this->GameBoard[0][7].piece = ROOK;
    
    this->GameBoard[0][4].ownership = PTWO;
    this->GameBoard[0][4].piece = KING;
    this->KingPositions[1] = &this->GameBoard[0][4];

    this->GameBoard[0][3].ownership = PTWO;
    this->GameBoard[0][3].piece = QUEEN;

    //! Important, we need to set the positions of all the remaining squares

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            GameBoard[row][col].pos = {col, row};
            //GameBoard[row][col].print();
        }
    }

}