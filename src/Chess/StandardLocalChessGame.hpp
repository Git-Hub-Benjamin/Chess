#include "TypesAndEnums/ChessEnums.hpp"
#include "TypesAndEnums/ChessTypes.hpp"
#include "StandardChessGame.hpp"
#include "ClientChessGame.hpp"
#include "Utils/TakenPiece.hpp"
#include "Utils/ChessClock.hpp"
#include "Utils/ChessConstants.hpp"

#include "Utils/ChessConstants.hpp"
#include "LegacyArray/GameSquare.hpp"
#include "LegacyArray/possibleMoveType.hpp"
#include "LegacyArray/LMove.hpp"
#include <vector>

class StandardLocalChessGame : private StandardChessGame, public ClientChessGame{

    //* LEGACY
    // Used to track the game board and kings' positions
    GameSquare GameBoard[CHESS_BOARD_HEIGHT][CHESS_BOARD_WIDTH];
    GameSquare* whitePlayerKing; // These pointers need to be set
    GameSquare* blackPlayerKing;
    GameSquare* pieceCausingKingCheck = nullptr; 

    // Vector to hold possible moves
    std::vector<possibleMoveType> possibleMoves; 
    // For move highlighting
    GameSquare* fromHighlightedPiece = nullptr;
    GameSquare* toHighlightedPiece = nullptr;

    bool LpopulatePossibleMoves(GameSquare& moveFrom);
    bool LpiecePresent(Point);
    bool LverifyMove(LMove& move);
    bool LverifyMove(LMove&& move);
    bool LrookClearPath(LMove& move);
    bool LbishopClearPath(LMove& move);
    bool LpawnMoveCheck(LMove& move);
    bool LunobstructedPathCheck(LMove& move);
    GameSquare* LcanDefendKing(std::vector<GameSquare*>& teamPieces);
    bool LkingSafe();
    bool LkingSafeAfterMove(GameSquare& to);
    bool LcheckMate();
    GameSquare& LconvertMove(std::string move);
    ChessEnums::ValidateGameSquareResult LvalidateGameSquare(GameSquare& square, ChessTypes::GetMoveType getMoveType);
    ChessEnums::PossibleMovesResult LreadPossibleMoves(GameSquare& to);
    void LstartGame();
    void LprintBoard();
    void LprintBoardWithMoves();
    GameSquare *LisolateFromInCheckMoves();
    ChessEnums::MakeMoveResult LmakeMove(LMove&& move);
    void LinitGame();
    bool LvalidateMoveset(LMove&);
    bool LonBoard(Point&);
    bool LcastlingCheck(LMove &, ChessTypes::CastlingType);


    // Regardless of whether or not legacy
    // Get move
    ChessEnums::GetMoveResult getMove(ChessTypes::GetMoveType) override;
    void currTurnChessClock(bool&, int, const std::string&) override;
    ChessEnums::GameOptionResult optionMenu(char);

    bool isLoadingState = false;
    //void loadGameState(StandardChessGameHistoryState&); -- Only going to implement for bitboard

    // Tracks the pieces that were taken during the game
    TakenPiece playerTakenPieces[STANDARD_CHESSGAME_PLAYER_COUNT][STANDARD_CHESSGAME_TEAM_PIECE_COUNT];
    
    // For game clock, Local only tho
    bool isClock = false;
;
    // For undo / redo turns -- Only going to implement for bitboard
    //std::stack<StandardChessGameHistoryState> undoTurn;
    //std::stack<StandardChessGameHistoryState> redoTurn;

public:

    bool DEV_MODE_ENABLE = false;
    void DEV_MODE_PRESET();
    StandardLocalChessGame(Options, ChessClock, ChessTypes::Player, bool);
    StandardLocalChessGame(Options, ChessTypes::Player, bool);
    StandardLocalChessGame() {}

    // Start a standard game
    void startGame();

};