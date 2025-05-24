#include <thread>

#include "../StandardLocalChessGame.hpp"
#include "../TypesAndEnums/ChessTypes.hpp"
#include "../TypesAndEnums/ChessEnums.hpp"
#include "../../Util/Terminal/Terminal.hpp"
#include "../Utils/ChessClock.hpp"
#include "../../Util/Terminal/TextPieceArt.hpp"

StandardLocalChessGame::StandardLocalChessGame(Options gOptions, ChessClock clock, ChessTypes::Player firstTurn, bool dev_mode)
    : DEV_MODE_ENABLE(dev_mode),
      isClock(true),
      StandardChessGame(ChessTypes::GameConnectivity::Local, clock)
{
    if (firstTurn == static_cast<ChessTypes::Player>(0))
        initTurn();
    else
        currentTurn = firstTurn;
    GameOptions = gOptions;
#ifdef LEGACY_ARRAY_GAMEBOARD
    blackPlayerKing = &GameBoard[0][3];
    whitePlayerKing = &GameBoard[7][3];
#endif
    if (DEV_MODE_ENABLE)
        DEV_MODE_PRESET();
    else
#ifdef LEGACY_ARRAY_GAMEBOARD
        LinitGame();
#else
        initGame();
#endif
}

StandardLocalChessGame::StandardLocalChessGame(Options gOptions, ChessTypes::Player firstTurn, bool dev_mode)
    : DEV_MODE_ENABLE(dev_mode), StandardChessGame(ChessTypes::GameConnectivity::Local)
{
    if (firstTurn == static_cast<ChessTypes::Player>(0))
        initTurn();
    else
        currentTurn = firstTurn;
    GameOptions = gOptions;
#ifdef LEGACY_ARRAY_GAMEBOARD
    blackPlayerKing = &GameBoard[0][3];
    whitePlayerKing = &GameBoard[7][3];
#endif
    if (DEV_MODE_ENABLE)
        DEV_MODE_PRESET();
    else
#ifdef LEGACY_ARRAY_GAMEBOARD
        LinitGame();
#else
        initGame();
#endif
}



// -1 Invalid
// 0 QUIT
// 1 CONTINUE
// 2 Undo (Game state change)
// 3 Redo (Game state change)
ChessEnums::GameOptionResult StandardLocalChessGame::optionMenu(char ch)
{
    if (!GameOptions.boardHistory && ch >= '3')
        ch += 2;

    ChessEnums::GameOptionResult res;

    switch (ch)
    {
    case '1':
        change_player_color_option();
        res = ChessEnums::GameOptionResult::CONTINUE;
        break;
    case '2':
        WChessPrint("Not implemented.");
        return ChessEnums::GameOptionResult::CONTINUE; // NOT IMPLEMENTED
    case '3':                                          // Undo
        /*
        if (!undoTurn.empty())
        {
            StandardChessGameHistoryState state = undoTurn.top();
            loadGameState(state);
            redoTurn.push(state);
            return ChessEnums::GameOptionResult::CONTINUE;
        }
        else
            WChessPrint(L"No moves to undo...");
        */
        return ChessEnums::GameOptionResult::CONTINUE;
    case '4': // Redo
        /*
        if (!redoTurn.empty())
        {
            StandardChessGameHistoryState state = redoTurn.top();
            loadGameState(state);
            undoTurn.push(state);
            return ChessEnums::GameOptionResult::CONTINUE;
        }
        else
            WChessPrint(L"No moves to redo...");
        */
        return ChessEnums::GameOptionResult::CONTINUE;
    case '5':
        return ChessEnums::GameOptionResult::CONTINUE; // NOT IMPLEMENTED
    case '6':
    case 'q':
        return ChessEnums::GameOptionResult::QUIT;
    default:
        return ChessEnums::GameOptionResult::Invalid;
    }

    if (ch == '1' || ch == '2')
    {
        if (SETTING_CHANGE_AFFECTS_CONFIG_FILE)
            overwrite_option_file();
        GameOptions = global_player_option; // Update the game options
    }

    return res;
}

#ifdef __linux__
//! Weirdest bug ever, this must be defined in the same file for it to work
class TimerNonCannonicalController
{
public:
    TimerNonCannonicalController()
    {
        // Store the original terminal settings
        tcgetattr(STDIN_FILENO, &old_tio);
        memcpy(&new_tio, &old_tio, sizeof(struct termios));
        new_tio.c_lflag &= ~(ICANON | ECHO);
        new_tio.c_cc[VMIN] = 1;
        new_tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    }

    ~TimerNonCannonicalController()
    {
        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    }

private:
    struct termios old_tio, new_tio;
};
#endif

// -1 Invalid
// 0 QUIT
// 1 CONTINUE
// 2 Undo (Game state change)
// 3 Redo (Game state change)
ChessEnums::GetMoveResult StandardLocalChessGame::getMove(ChessTypes::GetMoveType getMoveType)
{
    // Needs non canonical mode
    if (isClock || GameOptions.dynamicMoveHighlighting)
    {
#ifdef __linux__
        TimerNonCannonicalController terminalcontroller; // Set non-canonical mode

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1)
        {
            std::cerr << "Failed to create pipe" << std::endl;
            return ChessEnums::GetMoveResult::ERROR; // Adjust return as necessary
        }

        struct pollfd fds[2];
        fds[0].fd = fileno(stdin); // File descriptor for std::cin
        fds[0].events = POLLIN;
        fds[1].fd = pipe_fd[0]; // File descriptor for the read end of the pipe
        fds[1].events = POLLIN;

        bool validMoveForMoveHighlightingPrintBoard = false;

        bool inOptionMenu = false;
        bool stopTimerDisplay = false;
        bool clearingBufferFromArrowClick = false;
        std::thread clockThread;

        if (isClock)
            clockThread = std::thread(
                &StandardLocalChessGame::currTurnChessClock,
                this,
                std::ref(stopTimerDisplay),
                pipe_fd[1],
                std::wstring(getMoveType == ChessTypes::GetMoveType::From
                                 ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: "
                                                     : playerToString(currentTurn) +
                                                           L", You're in check! Move: ")
                             : !currTurnInCheck ? playerToString(currentTurn) + L", To: "
                                                : playerToString(currentTurn) +
                                                      L", You're in check! To: "));
        else {
            WChessPrint(getMoveType == ChessTypes::GetMoveType::From
                               ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: "
                                                   : playerToString(currentTurn) +
                                                         L", You're in check! Move: ")
                               : !currTurnInCheck ? playerToString(currentTurn) + L", To: "
                                                   : playerToString(currentTurn) +
                                                         L", You're in check! To: ")
            WChessPrintFlush();
        }

        while (true)
        {
            char ch;
            int ret = poll(fds, 2, -1); // Wait indefinitely until an event occurs
            if (ret > 0)
            {
                if (fds[1].revents & POLLIN)
                {
                    read(pipe_fd[0], &ch, 1);
                    if (clockThread.joinable())
                        clockThread.join();
                    return ChessEnums::GetMoveResult::TIMER_RAN_OUT;
                }
                if (fds[0].revents & POLLIN)
                {
                    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
                    if (bytes_read > 0)
                    {
                        if (clearingBufferFromArrowClick)
                        {
                            if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D')
                                clearingBufferFromArrowClick = false;
                        }
                        if (ch == 27)
                            clearingBufferFromArrowClick = true;
                        else if (ch == '\n')
                        {
                            if (inOptionMenu)
                            {
                                switch (optionMenu(inputBuffer[0]))
                                {
                                case ChessEnums::GameOptionResult::INVALID: // Invalid input
                                    continue;
                                case ChessEnums::GameOptionResult::QUIT: // Quit game
                                    return ChessEnums::GetMoveResult::QUIT;
                                case ChessEnums::GameOptionResult::CONTINUE: // Continue game
                                    inputBuffer.clear();
                                    inOptionMenu = false;

                                    if (isClock)
                                    {
                                        stopTimerDisplay = false; // reset the flag
                                        clockThread = std::thread(
                                            &StandardLocalChessGame::currTurnChessClock,
                                            this,
                                            std::ref(stopTimerDisplay),
                                            pipe_fd[1],
                                            std::wstring(
                                                getMoveType == ChessTypes::GetMoveType::From
                                                    ? (!currTurnInCheck
                                                           ? playerToString(currentTurn) +
                                                                 L", Move: "
                                                           : playerToString(currentTurn) +
                                                                 L", You're in check! Move: ")
                                                : !currTurnInCheck
                                                    ? playerToString(currentTurn) + L", To: "
                                                    : playerToString(currentTurn) +
                                                          L", You're in check! To: "));
                                    }

                                    WChessPrint(L"\033[2K\r");
                                    WChessPrint(getMoveType == ChessTypes::GetMoveType::From
                                            ? (!currTurnInCheck
                                                           ? playerToString(currentTurn) +
                                                                 L", Move: "
                                                           : playerToString(currentTurn) +
                                                                 L", You're in check! Move: ")
                                                    : !currTurnInCheck
                                                          ? playerToString(currentTurn) + L", To: "
                                                          : playerToString(currentTurn) +
                                                                L", You're in check! To: "
                                            + inputBuffer);
                                    WChessPrintFlush();
                                        continue;
                                case ChessEnums::GameOptionResult::UNDO: // Undo (unimplemented)
                                    continue;
                                case ChessEnums::GameOptionResult::REDO: // Redo (unimplemented)
                                    continue;
                                }
                            }
                            else
                            {
                                if (inputBuffer == L"re" && getMoveType == ChessTypes::GetMoveType::To)
                                    return ChessEnums::GetMoveResult::CHOOSE_MOVE_AGAIN;

                                switch (sanitizeGetMove(inputBuffer))
                                {
                                case ChessEnums::SanitizeGetMoveResult::Invalid: // Invalid move
                                    continue;
                                case ChessEnums::SanitizeGetMoveResult::Options: // Option menu
                                    if (isClock)
                                    { // Stop timer temporarily if enabled
                                        stopTimerDisplay = true;
                                        if (clockThread.joinable())
                                            clockThread.join();
                                    }
                                    inOptionMenu = true; // Mark that we are in the option menu
                                    if (GameOptions.boardHistory)
                                        WChessPrint(L"\n\n1. Change Colors\n"
                                            << L"2. Change Art\n"
                                            << L"3. Undo Turn\n"
                                            << L"4. Redo Turn\n"
                                            << L"5. Continue\n"
                                            << L"6. Quit\n"
                                            << L"\n--> ");
                                    else
                                        WChessPrint(L"\n\n1. Change Colors\n"
                                            << L"2. Change Art\n"
                                            << L"3. Continue\n"
                                            << L"4. Quit\n"
                                            << L"\n--> ");
                                    WChessPrintFlush();
                                    inputBuffer.clear(); // Delete buffer when entering option menu
                                    continue;
                                default: // Valid move
                                    break;
                                }
                            }

                            stopTimerDisplay = true;
                            if (clockThread.joinable())
                                clockThread.join();
                            WChessPrint(L"\n");
                            return ChessEnums::GetMoveResult::VALID; // Valid input for move
                        }
                        else if (ch == 127)
                        { // Backspace
                            if (!inputBuffer.empty())
                            {
                                inputBuffer.pop_back();
                                WChessPrint(L"\b \b"); 
                                WChessPrintFlush();// Handle backspace correctly
                            }
                        }
                        else
                        {
                            inputBuffer += ch; // Append character directly to the inputBuffer

                            if (inOptionMenu)
                                WChessPrint(inputBuffer);
                                WChessPrintFlush();
                        }
                    }
                }

                // Every time an event happens (keystroke) if move highlighting is enabled check for a new board
                // print, //! Maybe dont do this becasue movehighlighting maybe should be an option for lower
                // end computers //! This does a lot of computing

                //! THIS NEEDS TO BE UPDATED FOR BITBOARD IMPLEMENTATION
                if (GameOptions.dynamicMoveHighlighting)
                {
                    if (getMoveType == ChessTypes::GetMoveType::From)
                    {
                        if (sanitizeGetMove(inputBuffer) == ChessEnums::SanitizeGetMoveResult::Valid &&
                            LvalidateGameSquare(LconvertMove(inputBuffer, currentTurn),
                                                ChessTypes::GetMoveType::From) ==
                                ChessEnums::ValidateGameSquareResult::Valid)
                        {
                            // This move is valid (on the board) and belongs to the player
                            fromHighlightedPiece = &LconvertMove(inputBuffer, currentTurn);
                            if (LpopulatePossibleMoves(LconvertMove(inputBuffer, currentTurn)))
                            {
                                printBoardWithMoves(currentTurn);
                                validMoveForMoveHighlightingPrintBoard = true;
                            }
                            else
                                possibleMoves.clear(); // i dont think we need this?
                        }
                        else
                        {
                            if (validMoveForMoveHighlightingPrintBoard)
                            {
                                fromHighlightedPiece = nullptr;
                                validMoveForMoveHighlightingPrintBoard = false;
                                possibleMoves.clear();
                                printBoard(currentTurn);
                            }
                        }
                    }
                    else
                    {
                        if (sanitizeGetMove(inputBuffer) == ChessEnums::SanitizeGetMoveResult::Valid &&
                            LvalidateGameSquare(LconvertMove(inputBuffer, currentTurn),
                                                ChessTypes::GetMoveType::To) ==
                                ChessEnums::ValidateGameSquareResult::Valid)
                        {
                            toHighlightedPiece = &LconvertMove(inputBuffer, currentTurn);
                            validMoveForMoveHighlightingPrintBoard = true;
                            printBoardWithMoves(currentTurn);
                        }
                        else
                        {
                            if (validMoveForMoveHighlightingPrintBoard)
                            {
                                toHighlightedPiece = nullptr;
                                validMoveForMoveHighlightingPrintBoard = true;
                                printBoardWithMoves(currentTurn);
                            }
                        }
                    }
                }

                // If there is no clock then we need to print the new buffer to the screen

                if (!isClock && !inOptionMenu)
                { // Clear line and return cursor to the beginning
                    WChessPrint(L"\033[2K\r");
                    WChessPrint(getMoveType == ChessTypes::GetMoveType::From
                                ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: "
                                                    : playerToString(currentTurn) +
                                                          L", You're in check! Move: ")
                                : !currTurnInCheck ? playerToString(currentTurn) + L", To: "
                                                    : playerToString(currentTurn) +
                                                          L", You're in check! To: "
                                + inputBuffer);
                    WChessPrintFlush();
                }
                else if (inOptionMenu)
                {
                    WChessPrint(L"\033[2K\r");
                    WChessPrint(L"--> " + inputBuffer);
                    WChessPrintFlush();
                }
            }
        }
#elif _WIN32
        WChessPrint("Clock and move highlighting not yet supported for windows...\n");
        exit(EXIT_SUCCESS);
#endif
    }
    else
    {
        while (true)
        {
            if (getMoveType == ChessTypes::GetMoveType::From)
            {
                if (!currTurnInCheck) {
                    WChessPrint(playerToString(currentTurn).c_str()); WChessPrint(", Move: ");
                } else {
                    WChessPrint(playerToString(currentTurn).c_str()); WChessPrint(", You're in check! Move: ");
                }
            }
            else
            {
                if (!currTurnInCheck) {
                    WChessPrint(playerToString(currentTurn).c_str()); 
                    WChessPrint(", To: ");
                } else {
                    WChessPrint(playerToString(currentTurn).c_str()); WChessPrint(", You're in check! To: ");
                }
            }

            WChessInput(inputBuffer);

            auto moveResult = sanitizeGetMove(inputBuffer);
            switch (moveResult) {
                case ChessEnums::SanitizeGetMoveResult::Invalid: // Invalid
                    continue;
                case ChessEnums::SanitizeGetMoveResult::Options:
                    if (GameOptions.boardHistory)
                        WChessPrint("\n\n1. Change Colors\n2. Change Art\n3. Undo Turn\n4. Redo Turn\n5. Continue\n6. Quit\n \n--> ");
                    else 
                        WChessPrint("\n\n1. Change Colors\n2. Change Art\n3. Continue\n4. Quit\n \n--> ");
                    WChessPrintFlush();
                    inputBuffer.clear();
                    WChessInput(inputBuffer);
                    switch (optionMenu(inputBuffer[0])) {
                        case ChessEnums::GameOptionResult::Invalid: // Invalid input
                            continue;
                        case ChessEnums::GameOptionResult::QUIT: // Quit game
                            return ChessEnums::GetMoveResult::QUIT;
                        case ChessEnums::GameOptionResult::CONTINUE: // Continue game
                            inputBuffer.clear();
                            continue;
                        case ChessEnums::GameOptionResult::UNDO: // Undo (unimplemented)
                            continue;
                        case ChessEnums::GameOptionResult::REDO: // Redo (unimplemented)
                            continue;
                    }
                case ChessEnums::SanitizeGetMoveResult::ReEnterMove:
                    inputBuffer.clear();
                    return ChessEnums::GetMoveResult::ReEnterMove;
                default:
                    break;
                }
            break; // Continue to next getmove iteration
        }
        return ChessEnums::GetMoveResult::VALID;
    }
}

    
void StandardLocalChessGame::currTurnChessClock(bool &stop_display, int writePipeFd, const std::string &out)
{
    int &count = *(currentTurn == ChessTypes::Player::PlayerOne ? gameClock.getWhiteTimeAddr() : gameClock.getBlackTimeAddr());
    while (count >= 0 && !stop_display)
    {
        for (int i = 0; i < 10; i++)
        {
            if (stop_display || count == 0)
                break;

            WChessPrint("\033[2K\r"); // Clear line and return cursor to the beginning
            WChessPrint("Time: "); 
            WChessPrint(std::to_string(count).c_str()); 
            WChessPrint(" - "); 
            WChessPrint(out.c_str()); 
            WChessPrint(inputBuffer.c_str());
            WChessPrintFlush();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        count--;
    }

#ifdef __linux__
    if (count <= 0)
        write(writePipeFd, "1", 1); // telling main thread that the timer ran out of time
#endif
}

void StandardLocalChessGame::startGame()
{
#ifdef LEGACY_ARRAY_GAMEBOARD
    LstartGame();
    return;
#else
    WChessPrint("BITBOARD NOT IMPLEMENTED YET...\n");
    exit(EXIT_SUCCESS);
#endif
}

