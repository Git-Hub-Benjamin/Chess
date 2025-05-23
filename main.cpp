#include <iostream>
#include <cstdint>
#include <string.h>
#include <cwctype>
#include <windows.h>
#include "src/Chess/TypesAndEnums/ChessTypes.hpp"
#include "src/Chess/Utils/ChessConstants.hpp"
#include "src/Chess/TypesAndEnums/ChessEnums.hpp"
#include "src/Util/Terminal/Terminal.hpp"
#include "src/Chess/LegacyArray/GameSpaure.hpp"
#include "src/Util/Terminal/TextPieceArt.hpp"

static int playerOneConversion;
static int playerTwoConversion;

ChessTypes::GameConnectivity GameConnectivity = ChessTypes::GameConnectivity::Local;
GameSquare GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];

void LinitGame()
{
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            ChessTypes::GamePiece pieceToPut = ChessTypes::GamePiece::None;
            ChessTypes::Owner playerOwnerToPut = ChessTypes::Owner::None;

            if (row < (int)ChessTypes::YCoordinate::Six)
                playerOwnerToPut = ChessTypes::Owner::PlayerTwo;
            if (row > (int)ChessTypes::YCoordinate::Three)
                playerOwnerToPut = ChessTypes::Owner::PlayerOne;

            if (row == (int)ChessTypes::YCoordinate::One ||
                row == (int)ChessTypes::YCoordinate::Eight)
            {
                switch (col)
                {
                case (int)ChessTypes::XCoordinate::A:
                case (int)ChessTypes::XCoordinate::H:
                    pieceToPut = ChessTypes::GamePiece::Rook;
                    break;
                case (int)ChessTypes::XCoordinate::B:
                case (int)ChessTypes::XCoordinate::G:
                    pieceToPut = ChessTypes::GamePiece::Knight;
                    break;
                case (int)ChessTypes::XCoordinate::C:
                case (int)ChessTypes::XCoordinate::F:
                    pieceToPut = ChessTypes::GamePiece::Bishop;
                    break;
                case (int)ChessTypes::XCoordinate::D:
                    pieceToPut = ChessTypes::GamePiece::King;
                    break;
                case (int)ChessTypes::XCoordinate::E:
                    pieceToPut = ChessTypes::GamePiece::Queen;
                    break;
                }
            }

            if (row == (int)ChessTypes::YCoordinate::Two ||
                row == (int)ChessTypes::YCoordinate::Seven)
                pieceToPut = ChessTypes::GamePiece::Pawn;

            GameBoard[row][col] = GameSquare(playerOwnerToPut, pieceToPut, Point(col, row));

            if (pieceToPut ==ChessTypes::GamePiece::None) // This first move made only applies to pieces with actual pieces there
                GameBoard[row][col].setFirstMoveMade();
        }
    }
}


void LprintBoard()
{
    std::string board;
    board += "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n";
    board += "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        board += "\t\t\t";
        std::cout.flush();
        board += std::to_string(CHESS_BOARD_HEIGHT - row);
        board += " ";
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            board += "| ";

            board += TEXT_PIECE_ART_COLLECTION[GameBoard[row][col].getOwner() == ChessTypes::Owner::PlayerOne ? STD_PIECE_ART_P1 : STD_PIECE_ART_P2][static_cast<int>(GameBoard[row][col].getPiece())];
            if (GameBoard[row][col].getOwner() == ChessTypes::Owner::PlayerOne)
                set_terminal_color(BRIGHT_GREEN);
            else if(GameBoard[row][col].getOwner() == ChessTypes::Owner::PlayerTwo)
                set_terminal_color(BRIGHT_AQUA);

            set_terminal_color(DEFAULT);
            board += " ";
        }
        board += "| " + std::to_string(CHESS_BOARD_HEIGHT - row) + "\n";
        board += "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    }
    board += "\t\t\t    a   b   c   d   e   f   g   h\n";
    WChessPrint(board.c_str());
}




int main() {
    SetConsoleOutputCP(CP_UTF8);
    WChessPrint("Code page set to UTF-8.\n");




    LinitGame();
    LprintBoard();
/*
    while(true) {
        WChessPrint("Row --> ");
        std::wstring row; 
        std::wcin >> row;
        WChessPrint("Col --> ");
        std::wstring col;
        std::wcin >> col;

        if (row[0] == L'q' || col[0] == L'q')
            break;

        wchar_t piece;
        if (GameBoard[std::stoi(row)][std::stoi(col)].getOwner() == ChessTypes::Owner::None)
            piece = L' ';
        else if (GameBoard[std::stoi(row)][std::stoi(col)].getOwner() == ChessTypes::Owner::PlayerOne) {
            piece = TEXT_PIECE_ART_COLLECTION[playerOneConversion][static_cast<int>(GameBoard[std::stoi(row)][std::stoi(col)].getPiece())];
            set_terminal_color(DEFAULT);
        } else {
            piece = TEXT_PIECE_ART_COLLECTION[playerTwoConversion][static_cast<int>(GameBoard[std::stoi(row)][std::stoi(col)].getPiece())];
            set_terminal_color(DEFAULT);
        }

        WChessPrint("Printing piece: "); WChessPrint(&piece); WChessPrint("\n");
    }
*/
    return 0;
}