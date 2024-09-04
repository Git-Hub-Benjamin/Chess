#include "../chess.hpp"

StandardLocalChessGame::StandardLocalChessGame(Options gOptions, ChessClock clock, bool dev_mode)
    : DEV_MODE_ENABLE(dev_mode),
      isClock(true),
      StandardChessGame(LOCAL_CONNECTIVITY, clock)
{
    GameOptions = gOptions;
    blackPlayerKing = &GameBoard[0][3];
    whitePlayerKing = &GameBoard[7][3];
    initTurn();
    if (DEV_MODE_ENABLE)
        DEV_MODE_PRESET();
    else
        initGame();
}

StandardLocalChessGame::StandardLocalChessGame(Options gOptions, bool dev_mode)
    : DEV_MODE_ENABLE(dev_mode), StandardChessGame(LOCAL_CONNECTIVITY)
{
    GameOptions = gOptions;
    blackPlayerKing = &GameBoard[0][3];
    whitePlayerKing = &GameBoard[7][3];
    initTurn();
    if (DEV_MODE_ENABLE)
        DEV_MODE_PRESET();
    else
        initGame();
}

void StandardLocalChessGame::loadGameState(StandardChessGameHistoryState &state)
{

    std::wcout << "Printing state: " << std::endl;
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            state.mGameBoard[row][col].print();
            std::wcout << std::endl;
        }
    }
    std::wcout << "State white king: ";
    state.mGameBoard[state.mWhitePlayerKingPos.m_y][state.mWhitePlayerKingPos.m_x].print();
    std::wcout << std::endl;
    std::wcout << "State black king: ";
    state.mGameBoard[state.mBlackPlayerKingPos.m_y][state.mBlackPlayerKingPos.m_x].print();
    std::wcout << std::endl;
    std::wcout << "State piece causing king: ";
    if (state.mPieceCausingKingCheckPos.m_x == -1)
        std::wcout << "NONE - -1 -1, NULLPTR";
    else
        state.mGameBoard[state.mPieceCausingKingCheckPos.m_y][state.mPieceCausingKingCheckPos.m_x].print();
    std::wcout << std::endl;

    std::wcout << "State white time: " << state.mClock.getWhiteSeconds() << std::endl;
    std::wcout << "State black time: " << state.mClock.getBlackSeconds() << std::endl;

    memcpy(this->GameBoard, state.mGameBoard, sizeof(GameSquare[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH]));
    whitePlayerKing = &GameBoard[state.mWhitePlayerKingPos.m_y][state.mWhitePlayerKingPos.m_x];
    blackPlayerKing = &GameBoard[state.mBlackPlayerKingPos.m_y][state.mBlackPlayerKingPos.m_x];
    if (state.mPieceCausingKingCheckPos.m_x != -1 && state.mPieceCausingKingCheckPos.m_y != 1)
        pieceCausingKingCheck = &GameBoard[state.mPieceCausingKingCheckPos.m_y][state.mPieceCausingKingCheckPos.m_x];
    else
        pieceCausingKingCheck = nullptr;
    currentTurn = state.mCurrentTurn;
    if (isClock)
        gameClock = state.mClock;

    std::wcout << "Printing Chessgame after state load: " << std::endl;
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            GameBoard[row][col].print();
            std::wcout << std::endl;
        }
    }
    std::wcout << "Chessgame white king: ";
    whitePlayerKing->print();
    std::wcout << std::endl;
    std::wcout << "Chessgame black king: ";
    blackPlayerKing->print();
    std::wcout << std::endl;
    std::wcout << "Chessgame piece causing king: ";
    if (pieceCausingKingCheck == nullptr)
        std::wcout << "NONE - -1 -1, NULLPTR";
    else
        pieceCausingKingCheck->print();
    std::wcout << std::endl;

    std::wcout << "Chessgame white time: " << gameClock.getWhiteSeconds() << std::endl;
    std::wcout << "Chessgame black time: " << gameClock.getBlackSeconds() << std::endl;
}

// -1 Invalid
// 0 QUIT
// 1 CONTINUE
// 2 Undo (Game state change)
// 3 Redo (Game state change)

int StandardLocalChessGame::optionMenu(char ch)
{
    if (!GameOptions.boardHistory && ch >= '3')
        ch+=2;
    switch (ch)
    {
    case '1':
        std::wcout << L"Not implemented." << std::endl;
        return 1;
    case '2':
        std::wcout << L"Not implemented." << std::endl;
        return 1;
    case '3': // Undo
        if (!undoTurn.empty())
        {
            StandardChessGameHistoryState state = undoTurn.top();
            loadGameState(state);
            redoTurn.push(state);
            return 3;
        }
        else
        {
            std::wcout << "No moves to undo..." << std::endl;
        }
        return 1;
    case '4': // Redo
        std::wcout << L"Not implemented." << std::endl;
        return 1;
        if (!redoTurn.empty())
        {
            StandardChessGameHistoryState state = redoTurn.top();
            loadGameState(state);
            undoTurn.push(state);
            return 4;
        }
        else
        {
            std::wcout << "No moves to redo..." << std::endl;
        }
        return 1;
    case '5':
        return 1;
    case '6':
    case 'q':
        return 0;
    default:
        return -1;
    }
}

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

namespace getMove {
    enum getMove {
        ERROR = -1,
        QUIT,
        VALID,
        TIMER_RAN_OUT
    };
}

namespace sanitizeGetMove {
    enum sanitizeGetMove {
        INVALID = -1,
        OPTIONS,
        VALID
    };
}

// -1 Invalid
// 0 QUIT
// 1 CONTINUE
// 2 Undo (Game state change)
// 3 Redo (Game state change)

namespace optionMenu {
    enum optionMenu {
        INVALID = -1,
        QUIT,
        CONTINUE,
        UNDO,
        REDO
    };
}

int StandardLocalChessGame::getMove(int which) {

    // Needs non canonical mode
    if (isClock || GameOptions.moveHighlighting) {
        TimerNonCannonicalController terminalcontroller; // Set non-canonical mode
        
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            std::cerr << "Failed to create pipe" << std::endl;
            return getMove::ERROR; // Adjust return as necessary
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
            clockThread = std::thread(&StandardLocalChessGame::currTurnChessClock, this, std::ref(stopTimerDisplay), pipe_fd[1], std::wstring(which == 0 ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ") : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: "));
        else 
            std::wcout << (which == 0 ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ") : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: ") << std::flush;

        while (true) {
            char ch;
            int ret = poll(fds, 2, -1); // Wait indefinitely until an event occurs
            if (ret > 0) {
                if (fds[1].revents & POLLIN) {
                    read(pipe_fd[0], &ch, 1);
                    if (clockThread.joinable())
                        clockThread.join();
                    return getMove::TIMER_RAN_OUT;
                }
                if (fds[0].revents & POLLIN) {
                    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
                    if (bytes_read > 0)
                    {
                        if (clearingBufferFromArrowClick) {
                            if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D')
                                clearingBufferFromArrowClick = false;
                        }
                        if (ch == 27)
                            clearingBufferFromArrowClick = true;
                        else if (ch == '\n') {
                            if (inOptionMenu) {

                                switch (optionMenu(inputBuffer[0])) {
                                    case optionMenu::INVALID: // Invalid input
                                        continue; 
                                    case optionMenu::QUIT: // Quit game
                                        return getMove::QUIT;
                                    case optionMenu::CONTINUE: // Continue game
                                        inputBuffer.clear(); 
                                        inOptionMenu = false;

                                        if (isClock) {
                                            stopTimerDisplay = false; // reset the flag
                                            clockThread = std::thread(&StandardLocalChessGame::currTurnChessClock, this, std::ref(stopTimerDisplay), pipe_fd[1], std::wstring(which == 0 ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ") : !currTurnInCheck ? playerToString(currentTurn) + L", To: "
                                                                                                                                                                                                                                                                                                                                        : playerToString(currentTurn) + L", You're in check! To: "));
                                        }

                                        std::wcout << L"\033[2K\r";
                                        std::wcout << (which == 0 ? 
                                            (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ")
                                            : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: ") 
                                        << inputBuffer << std::flush;
                                        continue;
                                    case optionMenu::UNDO: // Undo (unimplemented)
                                        continue;
                                    case optionMenu::REDO: // Redo (unimplemented)
                                        continue;
                                }
                            } else {

                                switch(sanitizeGetMove(inputBuffer)) {
                                    case sanitizeGetMove::INVALID: // Invalid move
                                        continue; 
                                    case sanitizeGetMove::OPTIONS: // Option menu
                                        if (isClock) { // Stop timer temporarily if enabled 
                                            stopTimerDisplay = true;
                                            if (clockThread.joinable())
                                                clockThread.join();
                                        }
                                        inOptionMenu = true; // Mark that we are in the option menu
                                        if (GameOptions.boardHistory)
                                            std::wcout << L"\n\n1. Change Colors\n"
                                                        << L"2. Change Art\n"
                                                        << L"3. Undo Turn\n"
                                                        << L"4. Redo Turn\n"
                                                        << L"5. Continue\n"
                                                        << L"6. Quit\n"
                                                        << "\n--> " << std::flush;
                                        else
                                            std::wcout << L"\n\n1. Change Colors\n"
                                                        << L"2. Change Art\n"
                                                        << L"3. Continue\n"
                                                        << L"4. Quit\n"
                                                        << "\n--> " << std::flush;
                                        inputBuffer.clear(); // Delete buffer when entering option menu
                                        continue;
                                    default: // Valid move
                                        break; 
                                }
                            }

                            stopTimerDisplay = true;
                            if (clockThread.joinable())
                                clockThread.join();
                            std::wcout << std::endl;
                            return getMove::VALID;           // Valid input for move

                        } else if (ch == 127) { // Backspace
                            if (!inputBuffer.empty()) {
                                inputBuffer.pop_back();
                                std::wcout << L"\b \b" << std::flush; // Handle backspace correctly
                            }
                        } else {
                            inputBuffer += ch; // Append character directly to the inputBuffer

                            if (inOptionMenu)
                                std::wcout << inputBuffer << std::flush;
                        }
                    }
                }

                // Every time an event happens (keystroke) if move highlighting is enabled check for a new board print,
                //! Maybe dont do this becasue movehighlighting maybe should be an option for lower end computers
                //! This does a lot of computing

                if (sanitizeGetMove(inputBuffer) == 1 && validateGameSquare(convertMove(inputBuffer, currentTurn), FROM_MOVE) == 3) {
                    // This move is valid (on the board) and belongs to the player
                    if (populatePossibleMoves(convertMove(inputBuffer, currentTurn))) {
                        if (GameOptions.moveHighlighting)
                            printBoardWithMoves(currentTurn);
                        validMoveForMoveHighlightingPrintBoard = true;
                    } else
                        possibleMoves.clear(); // i dont think we need this?
                } else {
                    if (validMoveForMoveHighlightingPrintBoard) {
                        validMoveForMoveHighlightingPrintBoard = false;
                        possibleMoves.clear();
                        if (GameOptions.moveHighlighting)
                            printBoard(currentTurn);
                    }
                }

                // If there is no clock then we need to print the new buffer to the screen

                if (!isClock && !inOptionMenu) { // Clear line and return cursor to the beginning
                    std::wcout << L"\033[2K\r";
                    std::wcout << (which == 0 ? 
                        (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ")
                        : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: ") 
                    << inputBuffer << std::flush;
                } else if (inOptionMenu) {
                    std::wcout << L"\033[2K\r";
                    std::wcout << "--> " << inputBuffer << std::flush;
                }
            }
        }
    } else {
        while (true) {
            if (which == 0) {
                if (!currTurnInCheck)
                    std::wcout << playerToString(currentTurn) << L", Move: ";
                else
                    std::wcout << playerToString(currentTurn) << L", You're in check! Move: ";
            }
            else {
                if (!currTurnInCheck)
                    std::wcout << playerToString(currentTurn) << L", To: ";
                else
                    std::wcout << playerToString(currentTurn) << L", You're in check! To: ";
            }

            std::wcin >> inputBuffer;
            
            switch(sanitizeGetMove(inputBuffer)) {
                case sanitizeGetMove::INVALID: // Invalid
                    continue;
                case sanitizeGetMove::OPTIONS:
                    if (GameOptions.boardHistory)
                        std::wcout << L"\n\n1. Change Colors\n"
                                    << L"2. Change Art\n"
                                    << L"3. Undo Turn\n"
                                    << L"4. Redo Turn\n"
                                    << L"5. Continue\n"
                                    << L"6. Quit\n"
                                    << "\n--> " << std::flush;
                    else
                        std::wcout << L"\n\n1. Change Colors\n"
                                    << L"2. Change Art\n"
                                    << L"3. Continue\n"
                                    << L"4. Quit\n"
                                    << "\n--> " << std::flush;
                    inputBuffer.clear();
                    std::wcin >> inputBuffer;
                    switch (optionMenu(inputBuffer[0])) {
                        case optionMenu::INVALID: // Invalid input
                            continue; 
                        case optionMenu::QUIT: // Quit game
                            return getMove::QUIT;
                        case optionMenu::CONTINUE: // Continue game
                            inputBuffer.clear(); 
                            continue;
                        case optionMenu::UNDO: // Undo (unimplemented)
                            continue;
                        case optionMenu::REDO: // Redo (unimplemented)
                            continue;
                    }
                default:
                    break;
                }
            break; // Continue to next getmove iteration
        }
        return getMove::VALID;
    }
}

GameSquare *StandardLocalChessGame::isolateFromInCheckMoves()
{
    int potential_moves_to_get_out_of_check = kingCanMakeMove ? 1 : 0; // Start at 1 if kingCan make a move
    GameSquare *isolatedPiece = nullptr;
    std::vector<GameSquare *> teamPieces;

    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            GameSquare &gTemp = GameBoard[row][col];

            if (gTemp.getPiece() == KING)
                continue;

            if (static_cast<Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
        }
    }

    for (auto TeamPiece : teamPieces)
    {

        if (TeamPiece->getPiece() == QUEEN)
        {

            // pretend it is a rook temporarily
            TeamPiece->setPiece(ROOK);

            if (canDefendKing(teamPieces))
            {
                isolatedPiece = TeamPiece;
                potential_moves_to_get_out_of_check++;
            }
            else
            {
                // pretend it is bishop temporarily
                TeamPiece->setPiece(ROOK);
                if (canDefendKing(teamPieces))
                {
                    isolatedPiece = TeamPiece;
                    potential_moves_to_get_out_of_check++;
                }
            }
            pieceCausingKingCheck->setPiece(QUEEN);
        }
        else if (canDefendKing(teamPieces))
        {
            isolatedPiece = TeamPiece;
            potential_moves_to_get_out_of_check++;
        }

        if (potential_moves_to_get_out_of_check >= 2)
            return nullptr;
    }

    if (kingCanMakeMove)
        return currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing;
    return isolatedPiece;
}

void StandardLocalChessGame::currTurnChessClock(bool &stop_display, int writePipeFd, const std::wstring &out)
{
    int &count = *(currentTurn == PlayerOne ? gameClock.getWhiteTimeAddr() : gameClock.getBlackTimeAddr());
    while (count >= 0 && !stop_display)
    {
        for (int i = 0; i < 10; i++)
        {
            if (stop_display || count == 0)
                break;

            std::wcout << "\033[2K\r";  // Clear line and return cursor to the beginning
            std::wcout << L"\033[G" << L"Time: " << std::to_wstring(count) << " - " << out << inputBuffer;
            std::wcout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        count--;
    }
    if (count <= 0)
        write(writePipeFd, "1", 1); // telling main thread that the timer ran out of time
}

void StandardLocalChessGame::startGame()
{

    int game_loop_iteration = 0;

    StandardChessGameHistoryState oldState;

    while (!GameOver)
    {

        if (GameOptions.boardHistory)
        {

            if (isLoadingState)
                isLoadingState = false;
            else
            {
                if (isClock)
                    oldState = StandardChessGameHistoryState(GameBoard, currentTurn, whitePlayerKing, blackPlayerKing, pieceCausingKingCheck, gameClock);
                else
                    oldState = StandardChessGameHistoryState(GameBoard, currentTurn, whitePlayerKing, blackPlayerKing, pieceCausingKingCheck);
            }
        }

        std::wcout << "White Player King: ";
        whitePlayerKing->print();
        std::wcout << std::endl;
        std::wcout << "Black Player King: ";
        blackPlayerKing->print();
        std::wcout << std::endl;

        // Reset check
        currTurnInCheck = false;
        kingCanMakeMove = true;

        if (!kingSafe()) {
            if (checkMate()) {
                printBoard(currentTurn);
                GameOver = true;
                std::wcout << "GameOver!!" << std::endl;
                break;
            }
            else {
                currTurnInCheck = true;
            }
        }

        bool skipInitalPrintForMoveHighlighting = false;

        while (true) {

            // Clear the possibleMoves vec
            possibleMoves.clear();

            // This is for move highlighting, if the move returned from getMove was just typed or typed & entered
            bool enteredMove = true; // assume entered bc only 1 case its not
            std::wstring moveFrom;
            std::wstring moveTo;
            GameSquare *oneMoveFromCheck = nullptr;

            if (currTurnInCheck)
                oneMoveFromCheck = isolateFromInCheckMoves();

            // This will be the default case, if in check and ioslateFromInCheckMoves() returns a
            // pointer it means that there is only 1 possible move from check so we dont have to ask for a move
            if (oneMoveFromCheck == nullptr)
            {
                printBoard(currentTurn);

                int res = getMove(FROM_MOVE);
                moveFrom = inputBuffer;
                inputBuffer.clear();
                if (!res) {
                    // Quit
                    GameOver = true;
                    break;
                }
                else if (res == 2) {
                    // Timer ran out, end game, win for other player
                    std::wcout << "Timer ran out..." << std::endl;
                    GameOver = true;
                    break;
                }
                else if (GameOptions.boardHistory && (res == 3 || res == 4)) {
                    // Undoing turn
                    if (res == 3)
                        std::wcout << "Undoing turn..." << std::endl;
                    else
                        std::wcout << "Redoing turn..." << std::endl;
                    isLoadingState = true;
                    break;
                } 
    
                res = validateGameSquare(convertMove(moveFrom, currentTurn), FROM_MOVE);
                if (res == 0)
                {
                    if (enteredMove)
                        toPrint = L"No piece present.";
                    continue;
                }
                else if (res == 1)
                {
                    if (enteredMove)
                        toPrint = L"This piece does not belong to you.";
                    continue;
                }
            } else {
                populatePossibleMoves(*oneMoveFromCheck);
                printBoardWithMoves(currentTurn);
            }

            /*
            Still going to populate this even if piece highlighting is not enabled so we can print when a piece has no valid moves
            if (!populatePossibleMoves((oneMoveFromCheck == nullptr ? convertMove(moveFrom, currentTurn) : *oneMoveFromCheck))) {
                if (enteredMove) {
                    if (currTurnInCheck)
                        toPrint = L"You need to protect your king.";
                    else
                        toPrint = L"No moves with that piece.";
                    continue;
                }
            }
            */



            int res = getMove(TO_MOVE);
            moveTo = inputBuffer;
            inputBuffer.clear();
            if (!res) {
                // Quit
                GameOver = true;
                break;
            }
            else if (res == 2) {
                // Timer ran out, end game, win for other player
                std::wcout << "Timer ran out..." << std::endl;
                GameOver = true;
                break;
            }

            res = validateGameSquare(convertMove(moveTo, currentTurn), FROM_MOVE);
            if (res == 2)
            {
                toPrint = L"Cannot take your own piece.";
                continue;
            }

            int moveRes = makeMove(Move(oneMoveFromCheck == nullptr ? convertMove(moveFrom, currentTurn) : *oneMoveFromCheck, convertMove(moveTo, currentTurn)));
            if (moveRes == -1)
                toPrint = L"This puts your king in danger!";
            else if (moveRes == 0)
                toPrint = L"Invalid move.";
            else if (moveRes == 1)
                toPrint = L"Piece taken.";
            else
                toPrint = L"Piece moved.";

            if (moveRes < 1)
                continue;

            break;
        }

        if (!isLoadingState)
        {
            // Swap turns
            currentTurn = currentTurn == PlayerOne ? PlayerTwo : PlayerOne;

            // Loop iteration ++
            game_loop_iteration++;

            // Store undo turn
            if (GameOptions.boardHistory)
                undoTurn.push(oldState);
        }
    }
}
