// main2.cpp

#include <iostream>
#include <string>
#include <windows.h>
#include <stdint.h>

#define PIECE_ART_COLLECTION_AMOUNT 4
#define GAME_COLORS 8

#ifdef _WIN32

const char* TEXT_PIECE_ART_COLLECTION[4][7] = {
    { // White pieces
        " ",
        "\xE2\x99\x9F",
        "\xE2\x99\x9E",
        "\xE2\x99\x9D",
        "\xE2\x99\x9C",
        "\xE2\x99\x9A",
        "\xE2\x99\x9B",
    },
    {
        " ",
        "\xE2\x99\x99",
        "\xE2\x99\x98",
        "\xE2\x99\x97",
        "\xE2\x99\x96",
        "\xE2\x99\x94",
        "\xE2\x99\x95",
    },
    {
        " ",
        "P",
        "N",
        "B",
        "R",
        "K",
        "Q",
    },
    {
        " ",
        "p",
        "n",
        "b",
        "r",
        "k",
        "q",
    }
};

#else
const wchar_t* TEXT_PIECE_ART_COLLECTION[PIECE_ART_COLLECTION_AMOUNT] = {
    L" ♟♞♝♜♚♛",
    L" ♙♘♗♖♔♕",
    L" PNBRKQ",
    L" pnbrkq"
};
#endif


#ifdef _WIN32
// Windows (Ensuring UTF-8 is used & Chess Piece Art is displayed)
// 1. Download DejaVu Sans Mono font, Extract, Install
// 2. "Windows Key + R", type "regedit", press Enter
// 3. Navigate to HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
// 4. Right click, New, String Value, Name: 000, Value: DejaVu Sans Mono
// 5. Right click, Properties, Font, DejaVu Sans Mono
// 6. Apply, OK
// 7. Restart command prompt
// 8. "Windows Key + R", type "intl.cpl", press Enter
// 9. Administrative, Set system locale, Toggle "Beta: Use Unicode UTF-8 for worldwide language support"
// 10. Apply, OK, Restart Computer
void WChessPrint(const char* text){
    std::cout << text;
}

#else
void WChessPrint(const wchar_t* text){
     // On non-Windows, wcout with a UTF-8 locale is the way to go
     // setlocale(LC_ALL, "en_US.UTF-8"); // Might be needed on some systems
     std::wcout << text << std::endl;
}
#endif

int main() {
    SetConsoleOutputCP(65001);
    

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 7; j++) {
            WChessPrint(TEXT_PIECE_ART_COLLECTION[i][j]);
        }
        std::cout << std::endl;
    }
}
