#ifdef __linux__
    #include "sys/socket.h"
#elif _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
#endif

// Things that do not need to be exposed for the server side of chess game logic
class ClientChessGame {
protected:
    virtual ChessEnums::GetMoveResult getMove(ChessTypes::GetMoveType) = 0;
    // Function to handle current turn chess clock
    virtual void currTurnChessClock(bool& stop, int pipe, const std::wstring& msgToOutput) = 0;
};

class StandardOnlineChessGame : public StandardChessGame, public ClientChessGame {
    
    int* pipeFds; // For use by pipe and poll for timer and getting input in non cannonical mode
    int gameSocketFd;
    ChessTypes::Player playerNum;
    std::wstring oppossingPlayerString;

    //int makeMove(Move&&) override;
    //ChessEnums::GetMoveResult getMove(ChessTypes::getMoveType) override;
    
    int preTurnCheckIn();
    bool readyForNextTurn();
    int serverSaidValidMove();
    bool nonTurnSpecificCheckIn();
    bool verifyGameServerConnection();
    bool takeMovesAndSend(std::wstring, std::wstring);
    int notTurnRecieveMove(std::wstring& move, std::wstring& moveTo);
    void currTurnChessClock(bool&, int, const std::wstring&) override;


public:

    StandardOnlineChessGame(int, ChessTypes::Player, std::wstring);
    StandardOnlineChessGame(){}
    void startGame();

    ~StandardOnlineChessGame() { delete pipeFds; }
};