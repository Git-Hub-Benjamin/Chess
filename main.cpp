#include <iostream>
#include <cstdint>
#include <string.h>
#include <cwctype>
#include <windows.h>
#include "src/Chess/TypesAndEnums/ChessTypes.hpp"
#include "src/Chess/Utils/ChessConstants.hpp"
#include "src/Chess/TypesAndEnums/ChessEnums.hpp"
#include "src/Util/Terminal/Terminal.hpp"
#include "src/Chess/LegacyArray/GameSquare.hpp"
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
                setTerminalColor(BRIGHT_GREEN);
            else if(GameBoard[row][col].getOwner() == ChessTypes::Owner::PlayerTwo)
                setTerminalColor(BRIGHT_AQUA);

            setTerminalColor(DEFAULT);
            board += " ";
        }
        board += "| " + std::to_string(CHESS_BOARD_HEIGHT - row) + "\n";
        board += "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    }
    board += "\t\t\t    a   b   c   d   e   f   g   h\n";
    WChessPrint(board.c_str());
}



int LreflectAxis(int val)
{
    switch (val)
    {
    case 7:
        return 0;
    case 6:
        return 1;
    case 5:
        return 2;
    case 4:
        return 3;
    case 3:
        return 4;
    case 2:
        return 5;
    case 1:
        return 6;
    case 0:
        return 7;
    default:
        return -1;
    }
}


int main() {
    std::cout << LreflectAxis(0) << ", " << 7 - 0 << std::endl;
    std::cout << LreflectAxis(1) << ", " << 7 - 1 << std::endl;
    std::cout << LreflectAxis(2) << ", " << 7 - 2 << std::endl;
    std::cout << LreflectAxis(3) << ", " << 7 - 3 << std::endl;
    std::cout << LreflectAxis(4) << ", " << 7 - 4 << std::endl;
    std::cout << LreflectAxis(5) << ", " << 7 - 5 << std::endl;
    std::cout << LreflectAxis(6) << ", " << 7 - 6 << std::endl;
    std::cout << LreflectAxis(7) << ", " << 7 - 7 << std::endl;

    return 0;
}
