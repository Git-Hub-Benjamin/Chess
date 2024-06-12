#include "../../Chess/chess.hpp"
#include "../../server/server.hpp"

//! Weirdest bug ever, this must be defined in the same file for it to work
class TimerNonCannonicalControllerOnline { 
public:
    TimerNonCannonicalControllerOnline() {
        // Store the original terminal settings
        tcgetattr(STDIN_FILENO, &old_tio);
        memcpy(&new_tio, &old_tio, sizeof(struct termios));
        new_tio.c_lflag &= ~(ICANON | ECHO);
        new_tio.c_cc[VMIN] = 1;
        new_tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    }

    ~TimerNonCannonicalControllerOnline() {
        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    }

private:
    struct termios old_tio, new_tio;
};

void StandardOnlineChessGame::currTurnChessClock(std::atomic_bool& stop, int writePipe, const std::wstring& out) {
    int& count = *(currentTurn == PlayerOne ? gameClock.getWhiteTimeAddr() : gameClock.getBlackTimeAddr());
    int smallCount = count <= 60 ? count : 60;
    bool needSmallCount = count != smallCount; 
    while (smallCount >= 0 && !stop) {
        for (int i = 0; i < 10; i++) {

            if (stop)
                break;

            if (needSmallCount)
                std::wcout << L"\033[GTotal Time: " << count << L" - " << smallCount << " - " << out << inputBuffer << std::flush;
            else
                std::wcout << L"\033[GTotal Time: " << count << L" - " << out << inputBuffer << std::flush;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        smallCount--;
        count--;
    }

    if (smallCount >= 0)
        write(writePipe, "1", 1);
}

// True  --> No error
// False --> Error
bool StandardOnlineChessGame::takeMovesAndSend(std::wstring move, std::wstring to){
    std::string send_to_server = "move:";
    send_to_server += convertWString(move);
    send_to_server += "to:";
    send_to_server += convertWString(to);

    // Example:
    // move:b2to:b4

    if(send(gameSocketFd, (void*)send_to_server.c_str(), send_to_server.length(), 0) < 0)
        return false;
    return true;
}

// -1 Server sent data DC
// 0 - Server sent data Surrender
// 1 - Timer ran out of time
// 2 - All good
int StandardOnlineChessGame::getMove(int which) {

    std::atomic_bool stopTimerDisplay = false;
    TimerNonCannonicalControllerOnline terminalcontroller; // Set non-canonical mode

    struct pollfd fds[3];
    fds[0].fd = fileno(stdin); // File descriptor for std::cin
    fds[0].events = POLLIN;
    fds[1].fd = pipeFds[0]; // File descriptor for the read end of the pipe
    fds[1].events = POLLIN;
    fds[2].fd = gameSocketFd; // Listens for anything from the server
    fds[2].events = POLLIN;

    // Im not sure why I cant access currTurnChessClock
    std::thread clockThread(&StandardOnlineChessGame::currTurnChessClock, this, std::ref(stopTimerDisplay), pipeFds[1], std::wstring(
        which == 0 ? 
            (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ")
            : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: "
    ));

    // Listen for response from timer thread, 
    // Listen for events from poll for input 
    // Listen for data from the server

    while (true) {
        char ch;
        int ret = poll(fds, 3, -1); // Wait indefinitely until an event occurs
        if (ret > 0) {
            if (fds[2].revents & POLLIN) {

                stopTimerDisplay = true;

                if (clockThread.joinable())
                    clockThread.join();
                
                // If any data is sent from the server we need need to check if it was DC or Surrender
                char buffer[ONLINE_BUFFER_SIZE] = {0};
                size_t bytes_read = read(gameSocketFd, buffer, sizeof(buffer));
                std::string res(buffer);
                if (res == GAMESTATUS_GAMEOVER_DC)
                    return -1;
                else
                    return 0;

            }

            if (fds[1].revents & POLLIN) {

                // If either the chess clock or the 60 second small count run out of time we send the server we ran out of time

                if (clockThread.joinable())
                    clockThread.join();

                return 1;

            }
            if (fds[0].revents & POLLIN) {

                ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
                if (bytes_read > 0) {
                    if (ch == '\n') {
                        int sant_res = sanitizeGetMove(inputBuffer);
                        if (sant_res == -1)
                            continue; // Invalid

                        stopTimerDisplay = true;
                        if (clockThread.joinable())
                            clockThread.join();
                        return 2; // Valid input for move

                    } else if (ch == 127) { // Backspace
                        if (!inputBuffer.empty()) {
                            inputBuffer.pop_back();
                            std::wcout << L"\b \b" << std::flush; // Handle backspace correctly
                        }
                    } else {
                        inputBuffer += ch; // Append character directly to the inputBuffer
                    }
                }
            }
        }
    }

    if (clockThread.joinable())
        clockThread.join();

    return 2; //! Ran out of time
}

StandardOnlineChessGame::StandardOnlineChessGame(Options gOptions, int fd)
    : gameSocketFd(fd), StandardChessGame(ONLINE_CONNECTIVITY)
{
    GameOptions = gOptions;
}


// False -- Error
// True  -- Good
bool StandardOnlineChessGame::verifyGameServerConnection(){
    if(send(gameSocketFd, SERVER_CLIENT_ACK_MATCH_RDY, sizeof(SERVER_CLIENT_ACK_MATCH_RDY), 0) < 0)
        return false;
    return true;
}

// False -- Error
// True  -- Good
bool StandardOnlineChessGame::nonTurnSpecificCheckIn(){
    if(send(gameSocketFd, CLIENT_NON_TURN_CHECK_IN, sizeof(CLIENT_NON_TURN_CHECK_IN), 0) < 0)
        return false;
    return true;
}

// Server Error | -1
// Gameover  - 0
// Surrender - 1
// CheckMate - 2
// Check     - 3
// All good  - 4 
int StandardOnlineChessGame::preTurnCheckIn(){
    
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int byte_read = recv(gameSocketFd, (void*)buffer, sizeof(buffer), 0);
    
    if(byte_read <= 0)
        return -1;
    
    buffer[byte_read] = '\0';
    std::string res_from_server(buffer);

    if(res_from_server.compare(GAMESTATUS_GAMEOVER_DC) == 0)
        return 0; // again idk when this would occur
    
    if(res_from_server.compare(GAMESTATUS_SURRENDER) == 0)
        return 1;

    if(res_from_server.compare(GAMESTATUS_CHECKMATE) == 0)
        return 2;
    
    if(res_from_server.compare(GAMESTATUS_CHECK) == 0)
        return 3;

    return 4;
}

// -2 Server error
// -1 Puts self in check, so you cant make this move
// 0  Invalid move
// 1  Valid move
int StandardOnlineChessGame::serverSaidValidMove(){
    char buffer[ONLINE_BUFFER_SIZE] = {0};
    int bytes_read = recv(gameSocketFd, (void*)buffer, sizeof(buffer), 0);

    if(bytes_read <= 0)
        return -2;

    buffer[bytes_read] = '\0';
    std::string res_from_server(buffer);

    if(res_from_server.compare(SERVER_CLIENT_VERIFY_MOVE_VALID) == 0)
        return 1; // Valid move
    else if(res_from_server.compare(SERVER_CLIENT_VERIFY_MOVE_INVALID) == 0)
        return 0;
    return -1;
}

int StandardOnlineChessGame::makeMove(Move&& move) {
    bool pieceTake = false;

    if(move.getMoveTo().getPiece() != OPEN)
        pieceTake = true;

    // Lets move the piece now, This is also where we would do something different in case of castling since you are not setting the from piece to none / open
    move.getMoveTo().setPiece(move.getMoveFrom().getPiece());
    move.getMoveTo().setOwner(move.getMoveFrom().getOwner());
    move.getMoveFrom().setPiece(OPEN);
    move.getMoveFrom().setOwner(NONE);

    // Mark this gamesquare that a move has been made on this square
    move.getMoveFrom().setFirstMoveMade();

    if(pieceTake)
        return 1;
    else 
        return 2;
}

bool StandardOnlineChessGame::readyForNextTurn() {
    return send(gameSocketFd, (void*)CLIENT_RDY_FOR_NEXT_TURN, sizeof(CLIENT_RDY_FOR_NEXT_TURN), 0) < 0;
}

// -1 - Server Error
// 0 - Opponent DC
// 1 - Opponent Surrender 
// 2 - All good
int StandardOnlineChessGame::notTurnRecieveMove(std::wstring& move, std::wstring& moveTo){
    char buffer[ONLINE_BUFFER_SIZE] = {0};

    // This will block until the other player completes their turn

    std::wcout << "Blocking on the recv for the waiting non turn player" << std::endl;

    // Formatted: "move:()to:()"
    int bytesRead = recv(gameSocketFd, (void*)buffer, sizeof(buffer), 0);

    if (bytesRead <= 0)
        return -1;

    // So this will return the other players move and to, but if the player DC-ed
    // instead it will return an error message that they DC-ed

    buffer[bytesRead] = '\0';
    std::string res(buffer);

    if(res == GAMESTATUS_GAMEOVER_DC)
        return 0;
    else if(res == GAMESTATUS_SURRENDER)
        return 1;

    move += res[CLIENT_MOVE_INDEX_AFTER_COLON];
    move += res[CLIENT_MOVE_INDEX_AFTER_COLON + 1];
    moveTo += res[CLIENT_TO_INDEX_AFTER_COLON];
    moveTo += res[CLIENT_TO_INDEX_AFTER_COLON + 1];

    std::wcout << "Recieved from server for non turn, Move: " << move << ", MoveTo: " << moveTo << std::endl;

    return 2;
}

//! Update this so that generic functions like verifyGameServerConnection go into an interface class so that StandardOnlineChessGame inheirts from it and other online games for clients
void StandardOnlineChessGame::startGame() {

    if (!verifyGameServerConnection()) 
        return; // Socket failure

    while (!GameOver) {

        std::wstring moveFrom;
        std::wstring moveTo;

        int res = preTurnCheckIn();
        if (res == -1) {
            // Server Error
        } else if ( res < 3) {
            // End Game Message
            std::wstring endGameMsg;
            if(res == 0)
                endGameMsg = L"Gameover, connection problem.";
            else if(res == 1) {
                // Surrender
            } else {
                if(currentTurn == playerNum)
                    endGameMsg = L"Gameover, Checkmate, You lose!";
                else
                    endGameMsg = L"Gameover, Checkmate, You Win!";
            }
            std::wcout << endGameMsg << std::endl;
            return;
        } else if(res == 3)
            currTurnInCheck = true; // Check
        
        // Most of the time res = 4 will be sent which means, All is good, proceed


        if (playerNum == currentTurn) {

            while (true) {
                
                printBoard(playerNum);

                int res = getMove(FROM_MOVE);
                if (res == -1) {
                    // DC
                } else if (res == 0) {
                    // Surrender
                } else if (res == 1) {
                    // Timer ran out, tell server
                }
                moveFrom = inputBuffer;
                inputBuffer.clear();

                res = validateGameSquare(convertMove(moveFrom, playerNum), FROM_MOVE);
                if (res == 0) {
                    toPrint = L"No piece present.";
                    continue;
                } else if (res == 1) {
                    toPrint = L"This piece does not belong to you.";
                    continue;
                }
                if (!populatePossibleMoves(convertMove(moveFrom, playerNum))) {
                    if (currTurnInCheck) 
                        toPrint = L"You need to protect your king.";
                    else
                        toPrint = L"No moves with that piece.";
                    continue;
                }
                
                if (GameOptions.moveHighlighting) 
                    printBoardWithMoves(playerNum);

                int res = getMove(TO_MOVE);
                if (res == -1) {
                    // DC
                } else if (res == 0) {
                    // Surrender
                } else if (res == 1) {
                    // Timer ran out, tell server
                }
                moveTo = inputBuffer;
                inputBuffer.clear();

                res = validateGameSquare(convertMove(moveTo, playerNum), TO_MOVE);
                if (res == 2) {
                    toPrint = L"Cannot take your own piece.";
                    continue;
                }

                if (!takeMovesAndSend(moveFrom, moveTo))
                    return; // Handle socket error

                res = serverSaidValidMove();
                if (res == -2) {
                    return; // Server / Socket error
                } else if (res == -1) {
                    toPrint = L"This puts your king in check.";
                } else if (res == -2) {
                    toPrint = L"Invalid move.";
                }

                res = makeMove(Move(convertMove(moveFrom, playerNum), convertMove(moveTo, playerNum)));
                if(res == 1)
                    toPrint = L"Piece taken.";
                else
                    toPrint = L"Piece moved.";
                
                break;
                
            }

        } else {

            if (!nonTurnSpecificCheckIn)
                return; // Handle socket error
            
            int res = notTurnRecieveMove(moveFrom, moveTo);
            if (res == -1) {
                return; // Handle server error
            } else if (res == 0) {
                return; // Handle Opponent DC
            } else if (res == 1) {
                return; // Handle Opponent Surrender
            } 

            res = makeMove(Move(convertMove(moveFrom, (playerNum == PlayerOne ? PlayerTwo : PlayerOne)), convertMove(moveTo, (playerNum == PlayerOne ? PlayerTwo : PlayerOne))));
            if(res == 1)
                toPrint = L"Piece taken.";
            else
                toPrint = L"Piece moved.";
        }

        if (!readyForNextTurn())
            return; // Handle socket error

        // Swap turn
        currentTurn = currentTurn == PlayerOne ? PlayerTwo : PlayerOne;
    }
    

}