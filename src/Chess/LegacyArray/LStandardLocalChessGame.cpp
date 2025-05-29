#include "../StandardLocalChessGame.hpp"
#include "../TypesAndEnums/ChessTypes.hpp"
#include "../TypesAndEnums/ChessEnums.hpp"
#include "../../Util/Terminal/Terminal.hpp"
#include "../Utils/ChessClock.hpp"
#include "../../Util/Terminal/TextPieceArt.hpp"

#include "GameSquare.hpp"
#include "LMove.hpp"
#include "PossibleMoveType.hpp"
#include "Movesets.hpp"
#include "../Utils/ChessConstants.hpp"

bool StandardLocalChessGame::LonBoard(Point& p) {
    return (p.m_x <= 7 && p.m_x >= 0 && p.m_y <= 7 && p.m_y >= 0);
}


// Castling rules:
// 1. King and Rook must not have moved
// 2. King must not be in check
// 3. There must be no pieces between the king and the rook
// 4. King must not pass through a square that is attacked by an enemy piece
// 5. King must not end up in check
bool StandardLocalChessGame::LcastlingCheck(LMove &move, ChessTypes::CastlingType castlingType) {

    // 1.

    // making sure is king and moving to open square
    if (move.getMoveFrom().getPiece() != ChessTypes::GamePiece::King || move.getMoveTo().getPiece() != ChessTypes::GamePiece::None)
        return false;

    // making sure king is first move
    if (move.getMoveFrom().getIfFirstMoveMade())
        return false;

    // if moving right, making sure right rook is first move
    if (castlingType == ChessTypes::CastlingType::KingSide) // moving right
        if (currentTurn == ChessTypes::Player::PlayerOne)
            if (GameBoard[7][7].getPiece() != ChessTypes::GamePiece::Rook || GameBoard[7][7].getIfFirstMoveMade())
                return false;
        else
            if (GameBoard[0][7].getPiece() != ChessTypes::GamePiece::Rook || GameBoard[0][7].getIfFirstMoveMade())
                return false;
    else // moving left
        if (currentTurn == ChessTypes::Player::PlayerOne)
            if (GameBoard[7][0].getPiece() != ChessTypes::GamePiece::Rook || GameBoard[7][0].getIfFirstMoveMade())
                return false;
        else
            if (GameBoard[0][0].getPiece() != ChessTypes::GamePiece::Rook || GameBoard[0][0].getIfFirstMoveMade())
                return false;

    // 2.
    if (currTurnInCheck)
        return false;

    // 3. 4. 5. // checking the piece 

    int amount_to_check = castlingType == ChessTypes::CastlingType::KingSide ? 2 : 3; 

    for (int i = 1; i <= amount_to_check; i++) {
        Point temp = move.getMoveTo().getPosition();
        temp.m_x += (i * (castlingType == ChessTypes::CastlingType::KingSide ? 1 : -1));
        if (LpiecePresent(temp))
            return false;
        if (LkingSafeAfterMove(GameBoard[temp.m_y][temp.m_x]))
            return false;
    }

    if (!LkingSafeAfterMove(move.getMoveTo()))
        return false;

    // Verified all rules
    return true;
}

// True - There is at least one move
// False - No moves from this piece
bool StandardLocalChessGame::LpopulatePossibleMoves(GameSquare &moveFrom) {



    ChessTypes::GamePiece fromPiece = moveFrom.getPiece();
    short possibleMoveCounter = PIECE_MOVE_COUNTS[static_cast<int>(fromPiece) - 1];

    if (fromPiece == ChessTypes::GamePiece::Pawn && currentTurn == ChessTypes::Player::PlayerTwo)
        fromPiece = ChessTypes::GamePiece::None; // Might seem weird but this is just so that it indexes into the first array into pieceMovePtrs which is PawnDown

    for (int move_set_count = 0; move_set_count < possibleMoveCounter; move_set_count++)
    {

        // Iterating over entire moveset of a piece to see if it is

        // 1. on the board
        // 2. unobstructed path
        // 3. piece at square owner is not equal to current turn (except speical moves!)
        // 3. if its a king then making sure nothing can reach that square

        ChessEnums::PossibleMovesResult moveType = ChessEnums::PossibleMovesResult::NOT_FOUND; // default

        Point pTemp(moveFrom.getPosition().m_x + pieceMovePtrs[static_cast<int>(fromPiece)][move_set_count][0], moveFrom.getPosition().m_y + pieceMovePtrs[static_cast<int>(fromPiece)][move_set_count][1]);

        if (!LonBoard(pTemp))
            continue;

        LMove mTemp(moveFrom, GameBoard[pTemp.m_y][pTemp.m_x]);

        if (!currTurnInCheck) {

            if (mTemp.getMoveTo().getPiece() == ChessTypes::GamePiece::King)
                continue; // this makes sense, cannot capture king

            if (!LunobstructedPathCheck(mTemp))
                continue; // cannot move through pieces

            if (static_cast<ChessTypes::Player>(mTemp.getMoveTo().getOwner()) == currentTurn)
                continue; // cannot capture your own piece
            
            // if you are moving a king, then you need to make sure that it is safe after making that move
            if (mTemp.getMoveFrom().getPiece() == ChessTypes::GamePiece::King) {
                if (move_set_count == 8 || move_set_count == 9) { // if its the moveset of moving 2 squares then they might be castling
                    if (!LcastlingCheck(mTemp, move_set_count == 8 ? ChessTypes::CastlingType::KingSide : ChessTypes::CastlingType::QueenSide))
                        continue;
                    else
                        moveType = ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_SPECIAL_MOVE;
                }
                if (!LkingSafeAfterMove(mTemp.getMoveTo()))
                    continue;
            }

            // We have checked that this piece belongs to you (before this function), is on the board, unobstructed, not attacking yourself, and if a king then its safe after move
            possibleMoves.push_back(possibleMoveType(
                // pointer to the square that the piece COULD move to 
                &mTemp.getMoveTo(), 
                // type of move it is (just for color printing purposes), if moveType is its default value then just do the default behavior if not its castling or a special move
                moveType == ChessEnums::PossibleMovesResult::NOT_FOUND ? (mTemp.getMoveTo().getPiece() == ChessTypes::GamePiece::None ? ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_OPEN_SQAURE : ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_ENEMY_PIECE) : moveType));
            
        } else { 
            if (move_set_count == 8 || move_set_count == 9)
                continue; // cant be castling if in check

            // See if making this move would make the kingSafe()
            bool isKingMove = moveFrom.getPiece() == ChessTypes::GamePiece::King;
            GameSquare saveOldFrom(mTemp.getMoveFrom());
            GameSquare saveOldTo(mTemp.getMoveTo());

            mTemp.getMoveTo().setPiece(mTemp.getMoveFrom().getPiece());
            mTemp.getMoveTo().setOwner(mTemp.getMoveFrom().getOwner());
            mTemp.getMoveFrom().setPiece(ChessTypes::GamePiece::None);
            mTemp.getMoveFrom().setOwner(ChessTypes::Owner::None);

            // Making king move temporarily
            if (isKingMove) {
                if (currentTurn == ChessTypes::Player::PlayerOne){
                    whitePlayerKing = &mTemp.getMoveTo();
                } else {
                    blackPlayerKing = &mTemp.getMoveTo();
                }
            }

            if (LkingSafe())
                possibleMoves.push_back(possibleMoveType(&mTemp.getMoveTo(), isKingMove ? ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_KING_IN_DANGER : mTemp.getMoveTo().getPiece() == ChessTypes::GamePiece::None ? ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_PROTECT_KING_SQUARE : ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_PROTECT_KING_PIECE));

            // Revert
            mTemp.getMoveFrom().setPiece(saveOldFrom.getPiece());
            mTemp.getMoveFrom().setOwner(saveOldFrom.getOwner());
            mTemp.getMoveTo().setPiece(saveOldTo.getPiece());
            mTemp.getMoveTo().setOwner(saveOldTo.getOwner());

            // Revert king pos
            if (isKingMove){
                if (currentTurn == ChessTypes::Player::PlayerOne) {
                    whitePlayerKing = &mTemp.getMoveFrom();
                } else {
                    blackPlayerKing = &mTemp.getMoveFrom();
                }
            }
        }
    }
    return !possibleMoves.empty();
}

// True - Piece is present
// False - Piece is not present
bool StandardLocalChessGame::LpiecePresent(Point p)
{
    return !(GameBoard[p.m_y][p.m_x].getOwner() == ChessTypes::Owner::None && GameBoard[p.m_y][p.m_x].getPiece() == ChessTypes::GamePiece::None);
}

// True valid move
// False invalid move
bool StandardLocalChessGame::LverifyMove(LMove &move) { return LvalidateMoveset(move) && LunobstructedPathCheck(move); }

bool StandardLocalChessGame::LverifyMove(LMove &&move) { return LverifyMove(move); }

// True - valid moveset
// False - invalid moveset
bool StandardLocalChessGame::LvalidateMoveset(LMove& move) {
    ChessTypes::GamePiece fromPiece = move.getMoveFrom().getPiece();
    short possibleMoveCounter = PIECE_MOVE_COUNTS[static_cast<int>(fromPiece) - 1];

    if (fromPiece == ChessTypes::GamePiece::Pawn && currentTurn == ChessTypes::Player::PlayerTwo)
        fromPiece = ChessTypes::GamePiece::None; 

    for(int move_set_count = 0; move_set_count < possibleMoveCounter; move_set_count++)
        if (move.getMoveFrom().getPosition() + Point(pieceMovePtrs[static_cast<int>(fromPiece)][move_set_count][0], pieceMovePtrs[static_cast<int>(fromPiece)][move_set_count][1]) == move.getMoveTo().getPosition())
            return true;
    return false;
}

// True - Good, clear path
// False - Something blocking
bool StandardLocalChessGame::LrookClearPath(LMove &move)
{

    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    // Determine if moving along x or y axis
    if (from.m_x == to.m_x) {
        // moving along Y axis
        int amount_to_check = std::abs(from.m_y - to.m_y);
        int direction = (from.m_y - to.m_y < 0) ? 1 : -1;

        for (int i = 1; i < amount_to_check; i++) { //! Should these be <= ?
            struct Point temp = from;
            temp.m_y += (i * direction);
            if (LpiecePresent({temp.m_x, temp.m_y}))
                return false;
        }
    }
    else {
        // moving along X axis
        int amount_to_check = std::abs(from.m_x - to.m_x);
        int direction = (from.m_x - to.m_x < 0) ? 1 : -1;

        for (int i = 1; i < amount_to_check; i++) { //! Should these be <= ?
            struct Point temp = from;
            temp.m_x += (i * direction);
            if (LpiecePresent({temp.m_x, temp.m_y}))
                return false;
        }
    }
    return true;
}

// True - Good, clear path
// False - Something blocking
bool StandardLocalChessGame::LbishopClearPath(LMove &move)
{
    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    int xdir = (from.m_x - to.m_x < 0 ? 1 : -1);
    int ydir = (from.m_y - to.m_y < 0 ? 1 : -1);
    int amount_of_check = std::abs(from.m_x - to.m_x); // i dont think this should matter which one you do

    for (int i = 1; i < amount_of_check; i++)
    {
        struct Point temp = from;
        temp.m_x += (i * xdir);
        temp.m_y += (i * ydir);
        if (LpiecePresent({temp.m_x, temp.m_y}))
            return false;
    }
    return true;
}

// True - Valid move for pawn
// False - Invalid move for pawn
bool StandardLocalChessGame::LpawnMoveCheck(LMove &move)
{

    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    // Validates double advance and diagonal only capturing

    // Rules of double advanwcing for pawns
    // 1. Starting Position
    // 2. Unobstructed Path
    // 3. No Capturing

    if (std::abs(from.m_y - to.m_y) == 2)
    {

        // Check rule 2.
        if (std::abs(move.getMoveFrom().getPosition().m_y - move.getMoveTo().getPosition().m_y) == 2)
            if (LpiecePresent({from.m_x, from.m_y + (2 * (currentTurn == ChessTypes::Player::PlayerOne ? -1 : 1))}))
                return false;

        // Checking rule 1. and 3.
        if (move.getMoveFrom().getIfFirstMoveMade() || LpiecePresent({to.m_x, to.m_y}))
            return false;
    }
    else
    {
        // Now this section if the pawn is moving forward or diagonally

        if (from.m_x == to.m_x)
        {
            // moving forward 1, meaning it cant take pieces
            if (LpiecePresent({to.m_x, to.m_y}))
                return false;
        }
        else
        {

            // moving diagonally, meaning it has to take a piece to do this
            if (!LpiecePresent({to.m_x, to.m_y}))
                return false;
        }
    }

    return true; // Meaning this is a valid move
}

// True - All good
// False - Piece in way
bool StandardLocalChessGame::LunobstructedPathCheck(LMove &move)
{
    // Now verify if there is anything in path
    // Dont have to check knight bc it can go through pieces, also king can only go one piece

    switch (move.getMoveFrom().getPiece())
    {
    case (ChessTypes::GamePiece::Rook):
        return LrookClearPath(move);
    case (ChessTypes::GamePiece::Queen):
        if (move.getMoveFrom().getPosition().m_x == move.getMoveTo().getPosition().m_x || move.getMoveFrom().getPosition().m_y == move.getMoveTo().getPosition().m_y)
            return LrookClearPath(move);
        else
            return LbishopClearPath(move);
    case (ChessTypes::GamePiece::Bishop):
        return LbishopClearPath(move);
    case (ChessTypes::GamePiece::Pawn):
        return LpawnMoveCheck(move);
    default:
        return true; // Doesnt matter for (KNIGHT, KING & OPEN/NONE)
    }
}

// True - can defend the king so not checkmate
// False - checkmate!
// if it is a queen causing check, called will temporarily change piece causing check to rook, then change it to bishop then change it back
GameSquare *StandardLocalChessGame::LcanDefendKing(std::vector<GameSquare *> &teamPieces)
{

    Point pieceCausingCheckPos = pieceCausingKingCheck->getPosition();
    ChessTypes::GamePiece pieceCausingCheckPiece = pieceCausingKingCheck->getPiece();
    Point checkedKingPos = (currentTurn == ChessTypes::Player::PlayerOne ? whitePlayerKing : blackPlayerKing)->getPosition();

    int xdir = 0;
    int ydir = 0;
    int amount_to_check;

    if (pieceCausingCheckPiece == ChessTypes::GamePiece::Rook)
    {
        // Moving along Y axis
        if (pieceCausingCheckPos.m_x == checkedKingPos.m_x)
            ydir = (pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0) ? 1 : -1;
        else // Moving along X axis
            ydir = (pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0) ? 1 : -1;
        // Get amount
        amount_to_check = xdir == 0 ? std::abs(checkedKingPos.m_y - pieceCausingCheckPos.m_y) : std::abs(checkedKingPos.m_x - pieceCausingCheckPos.m_x);
    }
    else if (pieceCausingCheckPiece == ChessTypes::GamePiece::Bishop)
    {
        xdir = pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0 ? -1 : 1;
        ydir = pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0 ? -1 : 1;
        amount_to_check = std::abs(pieceCausingCheckPos.m_x - checkedKingPos.m_x);
    }

    if (pieceCausingCheckPiece == ChessTypes::GamePiece::Rook || pieceCausingCheckPiece == ChessTypes::GamePiece::Bishop)
    {
        for (int i = 1; i <= amount_to_check; i++)
        {

            // Iterate over teamPieces to see if they can reach the currentSquare
            for (GameSquare *teamPiece : teamPieces)
            {
                if (LverifyMove(LMove(*teamPiece, GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x])))
                {
                    return &GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x]; // Some team piece can take or block the enemy piece that is causing the check on the king
                }
            }

            pieceCausingCheckPos.m_x += (1 * xdir);
            pieceCausingCheckPos.m_y += (1 * ydir);
        }
    }
    else
    {
        for (auto teamPiece : teamPieces)
        {
            if (LverifyMove(LMove(*teamPiece, *pieceCausingKingCheck)))
                return pieceCausingKingCheck; // Some team piece can attack the knight, no gameover
        }
    }

    return nullptr; // Meaning we could not find a move to make to save the king
}


// True - King safe
// False - King NOT safe
bool StandardLocalChessGame::LkingSafe()
{

    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {

            GameSquare &curr = GameBoard[row][col];

            // If empty square we dont need to check, if current square is owned by
            // current trun then we dont have to check it
            if (curr.getOwner() == ChessTypes::Owner::None || static_cast<ChessTypes::Player>(curr.getOwner()) == currentTurn)
                continue;

            if (LverifyMove(LMove(curr, *(currentTurn == ChessTypes::Player::PlayerOne ? whitePlayerKing : blackPlayerKing))))
            {
                pieceCausingKingCheck = &curr;
                return false;
            }
        }
    }

    return true;
}

bool StandardLocalChessGame::LkingSafeAfterMove(GameSquare &to)
{

    GameSquare &currKing = *(currentTurn == ChessTypes::Player::PlayerOne ? whitePlayerKing : blackPlayerKing);
    currKing.setOwner(ChessTypes::Owner::None);
    currKing.setPiece(ChessTypes::GamePiece::None);

    bool res = true;

    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {

            GameSquare &curr = GameBoard[row][col];

            if (curr.getOwner() == ChessTypes::Owner::None || static_cast<ChessTypes::Player>(curr.getOwner()) == currentTurn)
                continue;

            if (LverifyMove(LMove(curr, to)))
            {
                res = false;
                goto restore_king;
            }
        }
    }
restore_king:
    currKing.setOwner(static_cast<ChessTypes::Owner>(currentTurn));
    currKing.setPiece(ChessTypes::GamePiece::King);

    return res;
}

// True Gameover, Current turn loses
// False the king can get out of check
bool StandardLocalChessGame::LcheckMate()
{ // Checking everything around the king

    GameSquare &kingToCheckSafteyFor = *(currentTurn == ChessTypes::Player::PlayerOne ? whitePlayerKing : blackPlayerKing);
    short kingPossibleMoves = KING_POSSIBLE_MOVES; // 8
    std::vector<GameSquare *> teamPieces;
    std::vector<GameSquare *> enemyPieces;

    // Getting enemy and team pieces
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            GameSquare &gTemp = GameBoard[row][col];
            if (gTemp.getOwner() == ChessTypes::Owner::None)
                continue;

            if (gTemp.getPiece() == ChessTypes::GamePiece::King)
                continue;

            else if (static_cast<ChessTypes::Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
            else
                enemyPieces.push_back(&gTemp);
        }
    }

    // Check each square around the King, if its not on the board SKIP it
    // If its taken by a teammate then SKIP it, bc the king cant take own piece
    //! If its taken by an oppnent then we need to check if its a pawn OR knight

    // Since we are checking these pieces assuming the king is moving there we need to also assume the king has moved
    kingToCheckSafteyFor.setOwner(ChessTypes::Owner::None);
    kingToCheckSafteyFor.setPiece(ChessTypes::GamePiece::None);

    bool res = true;

    for (int move_set_count = 0; move_set_count < kingPossibleMoves; move_set_count++)
    {

        Point currKingPosAroundKing(kingToCheckSafteyFor.getPosition().m_x + pieceMovePtrs[static_cast<int>(ChessTypes::GamePiece::King)][move_set_count][0], kingToCheckSafteyFor.getPosition().m_y + pieceMovePtrs[static_cast<int>(ChessTypes::GamePiece::King)][move_set_count][1]);

        if (!LonBoard(currKingPosAroundKing))
            continue;

        // ! You would need to check here if the king can castle to get out of danger
        if (static_cast<ChessTypes::Player>(GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x].getOwner()) == currentTurn)
            continue;

        GameSquare &currSquareAroundKingCheck = GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x];
        bool ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = false;

        for (auto enemy : enemyPieces)
        {

            if (LverifyMove(LMove(*enemy, currSquareAroundKingCheck)))
            {
                ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = true;
                break; // No point to keep checking, we know this square is NOT safe
            }
        }

        if (!ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE)
        {
            res = false;
            goto restore_king;
        }
    }

restore_king:

    kingToCheckSafteyFor.setOwner(static_cast<ChessTypes::Owner>(currentTurn));
    kingToCheckSafteyFor.setPiece(ChessTypes::GamePiece::King);

    if (!res)
        return false;

    kingCanMakeMove = false;

    // Check if any of the teampieces can defend the king by either taking the piece causing check OR blocking its path

    if (pieceCausingKingCheck->getPiece() == ChessTypes::GamePiece::Queen)
    {

        // pretend it is a rook temporarily
        pieceCausingKingCheck->setPiece(ChessTypes::GamePiece::Rook);

        if (LcanDefendKing(teamPieces))
        {
            pieceCausingKingCheck->setPiece(ChessTypes::GamePiece::Queen);
            return false;
        }

        // pretend it is bishop temporarily
        pieceCausingKingCheck->setPiece(ChessTypes::GamePiece::Rook);

        bool res = LcanDefendKing(teamPieces);

        pieceCausingKingCheck->setPiece(ChessTypes::GamePiece::Queen);

        return res;
    }

    return LcanDefendKing(teamPieces) == nullptr; // If any of the team pieces can defend the king then this will result in NO checkmate, otherwise checkmate
}

GameSquare &StandardLocalChessGame::LconvertMove(std::string move)
{
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top

    int row = 8 - (move[1] - 48);
    int col = move[0] - 97;

    if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) ||
        (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
        return GameBoard[7 - row][7 - col];

    return GameBoard[row][col];
}

// No Piece Present - 0
// This Piece Does not belong to you - 1
// Cannot take your own piece - 2
// Valid - 3
ChessEnums::ValidateGameSquareResult StandardLocalChessGame::LvalidateGameSquare(GameSquare &square, ChessTypes::GetMoveType getMoveType)
{
    if (getMoveType == ChessTypes::GetMoveType::From)
    {

        if (square.getOwner() == ChessTypes::Owner::None)
            return ChessEnums::ValidateGameSquareResult::NO_PIECE;

        if (static_cast<ChessTypes::Player>(square.getOwner()) != currentTurn)
            return ChessEnums::ValidateGameSquareResult::PIECE_NOT_YOURS;
    }
    else
    {
        if (static_cast<ChessTypes::Player>(square.getOwner()) == currentTurn)
            return ChessEnums::ValidateGameSquareResult::CANNOT_TAKE_OWN;
    }

    return ChessEnums::ValidateGameSquareResult::VALID;
}

// True found matching move with possibleMoves
// False not found
ChessEnums::PossibleMovesResult StandardLocalChessGame::LreadPossibleMoves(GameSquare &to)
{
    for (possibleMoveType &possibleMove : possibleMoves)
        if (to == *possibleMove.m_boardSquare)
            return possibleMove.possibleMoveTypeSelector;
    return ChessEnums::PossibleMovesResult::NOT_FOUND;
}

void StandardLocalChessGame::LstartGame()
{

    int game_loop_iteration = 0;

    while (!GameOver)
    {

        //^ Fyi - Gamestate has been purged from Legacy game forever

        // Reset check
        currTurnInCheck = false;
        kingCanMakeMove = true;

        if (!LkingSafe())
        {
            if (LcheckMate())
            {
                LprintBoard();
                GameOver = true;
                WChessPrint("GameOver!!\n");
                break;
            }
            else
            {
                currTurnInCheck = true;
            }
        }

        while (true)
        {

            possibleMoves.clear();          // Clear the possibleMoves vec
            fromHighlightedPiece = nullptr; // Make sure this is not set
            toHighlightedPiece = nullptr;   // Make sure this is not set
            inputBuffer.clear();            // Make sure this is empty

            std::string moveFrom;
            std::string moveTo;
            GameSquare *oneMoveFromCheck = nullptr;

            if (currTurnInCheck)
                oneMoveFromCheck = LisolateFromInCheckMoves();

            if (oneMoveFromCheck == nullptr)
            {
                LprintBoard();

                int res = static_cast<int>(getMove(ChessTypes::GetMoveType::From));

                if (static_cast<ChessEnums::GetMoveResult>(res) == ChessEnums::GetMoveResult::QUIT) { // Quit
                    GameOver = true;
                    break;
                } else if (static_cast<ChessEnums::GetMoveResult>(res) == ChessEnums::GetMoveResult::TIMER_RAN_OUT) { // Timer ran out, end game, win for other player
                    WChessPrint("Timer ran out...\n");
                    GameOver = true;
                    break;
                }

                moveFrom = inputBuffer;
                inputBuffer.clear();

                res = static_cast<int>(LvalidateGameSquare(LconvertMove(moveFrom), ChessTypes::GetMoveType::From));
                if (static_cast<ChessEnums::ValidateGameSquareResult>(res) == ChessEnums::ValidateGameSquareResult::NO_PIECE)
                {
                    toPrint = "No piece present.\n";
                    continue;
                }
                else if (static_cast<ChessEnums::ValidateGameSquareResult>(res) == ChessEnums::ValidateGameSquareResult::PIECE_NOT_YOURS)
                {
                    toPrint = "This piece does not belong to you.\n";
                    continue;
                }

                // We need to populate possible moves regardless of whether or not we have move highlighing,
                // if dynamic move highlighting is enabled this is already done in getMove
                if (!GameOptions.dynamicMoveHighlighting)
                {
                    if (!LpopulatePossibleMoves(LconvertMove(moveFrom)))
                    {
                        if (currTurnInCheck)
                            toPrint = "You need to protect your king.\n";
                        else
                            toPrint = "No moves with that piece.\n";
                        continue;
                    }
                    if (GameOptions.moveHighlighting)
                        LprintBoardWithMoves();
                }
            }
            else
            {
                if (!LpopulatePossibleMoves(*oneMoveFromCheck))
                    //! FATAL ERROR THIS SHOULD NOT HAPPEN
                    LprintBoardWithMoves();
            }

            // --------------------------------- //

            int res = static_cast<int>(getMove(ChessTypes::GetMoveType::To));

            if (static_cast<ChessEnums::GetMoveResult>(res) == ChessEnums::GetMoveResult::QUIT) { // Quit
                GameOver = true;
                break;
            } else if (static_cast<ChessEnums::GetMoveResult>(res) == ChessEnums::GetMoveResult::TIMER_RAN_OUT) { // Timer ran out, end game, win for other player
                WChessPrint("Timer ran out...\n");
                GameOver = true;
                break;
            } else if (static_cast<ChessEnums::GetMoveResult>(res) == ChessEnums::GetMoveResult::ReEnterMove)
                    continue;

            moveTo = inputBuffer;

            res = static_cast<int>(LvalidateGameSquare(LconvertMove(moveTo), ChessTypes::GetMoveType::To));
            if (static_cast<ChessEnums::ValidateGameSquareResult>(res) == ChessEnums::ValidateGameSquareResult::CANNOT_TAKE_OWN){
                toPrint = "Cannot take your own piece.\n";
                continue;
            }


            // ---------------------------------- //

            res = static_cast<int>(LmakeMove(LMove(oneMoveFromCheck == nullptr ? LconvertMove(moveFrom) : *oneMoveFromCheck, LconvertMove(moveTo))));
            if (static_cast<ChessEnums::MakeMoveResult>(res) == ChessEnums::MakeMoveResult::KingInDanger)
                toPrint = "This puts your king in danger!\n";
            else if (static_cast<ChessEnums::MakeMoveResult>(res) == ChessEnums::MakeMoveResult::InvalidMove)
                toPrint = "Invalid move.\n";
            else if (static_cast<ChessEnums::MakeMoveResult>(res) == ChessEnums::MakeMoveResult::PieceTaken)
                toPrint = "Piece taken.\n";
            else
                toPrint = "Piece moved.\n";

            if (res < 1)
                continue; // Redo turn

            break; // Next turn
        }

        // Swap turns
        currentTurn = currentTurn == ChessTypes::Player::PlayerOne ? ChessTypes::Player::PlayerTwo : ChessTypes::Player::PlayerOne;

        // Loop iteration ++
        game_loop_iteration++;
    }
}

//* may just override in the future, but having L "giving itself its own function" is fine for now
void StandardLocalChessGame::LprintBoard()
{
    if (GameOptions.clearScreenOnPrint) 
        eraseDisplay();
    setTerminalColor(DEFAULT);
#ifdef _WIN32  
    WChessPrint("\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n");
    WChessPrint("\t\t\t  +---+---+---+---+---+---+---+---+\n");
#else 
    WChessPrint(L"\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n");
    WChessPrint(L"\t\t\t  +---+---+---+---+---+---+---+---+\n");
#endif    
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
#ifdef _WIN32
        WChessPrint("\t\t\t"); WChessPrint(std::to_string(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint(" ");
#else 
        WChessPrint(L"\t\t\t"); WChessPrint(std::to_wstring(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint(L" ");
#endif
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
#ifdef _WIN32
            WChessPrint("| ");
#else 
            WChessPrint(L"| ");
#endif
            GameSquare* currSquare;

            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
                currSquare = &GameBoard[7 - row][7 - col];
            else
                currSquare = &GameBoard[row][col];

            setTerminalColor((*currSquare).getOwner() == ChessTypes::Owner::PlayerOne ? GameOptions.p1_color : GameOptions.p2_color);
            WChessPrint(TEXT_PIECE_ART_COLLECTION[(*currSquare).getOwner() == ChessTypes::Owner::PlayerOne ? GameOptions.whitePlayerArtSelector : GameOptions.blackPlayerArtSelector][static_cast<int>((*currSquare).getPiece())]);
            setTerminalColor(DEFAULT);
#ifdef _WIN32
            WChessPrint(" ");
#else 
            WChessPrint(L" ");
#endif
        }
#ifdef _WIN32
        WChessPrint("| "); WChessPrint(std::to_string(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint("\n");
        WChessPrint("\t\t\t  +---+---+---+---+---+---+---+---+\n");
#else 
        WChessPrint(L"| "); WChessPrint(std::to_wstring(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint(L"\n");
        WChessPrint(L"\t\t\t  +---+---+---+---+---+---+---+---+\n");
#endif
    }
#ifdef _WIN32
    WChessPrint("\t\t\t    a   b   c   d   e   f   g   h\n");
#else 
    WChessPrint(L"\t\t\t    a   b   c   d   e   f   g   h\n");
#endif

    if (!toPrint.empty()) {
#ifdef _WIN32
        WChessPrint(toPrint.c_str());
#else 
        WChessPrint(std::wstring(toPrint.begin(), toPrint.end()).c_str());
#endif
        toPrint.clear();
    }
}

//! Not working currently...
void StandardLocalChessGame::LprintBoardWithMoves()
{
    if (GameOptions.clearScreenOnPrint)
        eraseDisplay();
#ifdef _WIN32  
    WChessPrint("\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n");
    WChessPrint("\t\t\t  +---+---+---+---+---+---+---+---+\n");
#else 
    WChessPrint(L"\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n");
    WChessPrint(L"\t\t\t  +---+---+---+---+---+---+---+---+\n");
#endif   
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
#ifdef _WIN32
        WChessPrint("\t\t\t"); WChessPrint(std::to_string(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint(" ");
#else 
        WChessPrint(L"\t\t\t"); WChessPrint(std::to_wstring(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint(L" ");
#endif
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
#ifdef _WIN32
            WChessPrint("| ");
            std::string piece;
#else 
            WChessPrint(L"| ");
            std::wstring piece;
#endif
            ChessEnums::PossibleMovesResult possibleMoveTypeSelector;
            GameSquare* currSquare;
            WRITE_COLOR color = DEFAULT;

            if ((GameConnectivity == ChessTypes::GameConnectivity::Online && currentTurn == ChessTypes::Player::PlayerTwo) || (GameConnectivity == ChessTypes::GameConnectivity::Local && GameOptions.flipBoardOnNewTurn && currentTurn == ChessTypes::Player::PlayerTwo))
                currSquare = &GameBoard[7 - row][7 -col];
            else
                currSquare = &GameBoard[row][col];

            piece = TEXT_PIECE_ART_COLLECTION[(*currSquare).getOwner() == ChessTypes::Owner::PlayerOne ? GameOptions.whitePlayerArtSelector : GameOptions.blackPlayerArtSelector][static_cast<int>((*currSquare).getPiece())];
      
            possibleMoveTypeSelector = LreadPossibleMoves(*currSquare);

            if (possibleMoveTypeSelector != ChessEnums::PossibleMovesResult::NOT_FOUND) {
#ifdef _WIN32
                if (piece == " ")
                    piece = "X";
#else 
                if (piece == L" ")
                    piece = L"X";
#endif
                switch (possibleMoveTypeSelector) {
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_ENEMY_PIECE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_OPEN_SQAURE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_PROTECT_KING_PIECE:
                        color = WRITE_COLOR::BRIGHT_MAGENTA;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_PROTECT_KING_SQUARE:
                        color = WRITE_COLOR::BRIGHT_YELLOW;
                        break;
                    case ChessEnums::PossibleMovesResult::POSSIBLE_MOVE_KING_IN_DANGER:
                        color = WRITE_COLOR::YELLOW;
                        break;
                    default:
                        color = WRITE_COLOR::YELLOW; // for debugging because idk when this would happen
                        break;
                }
            }

            // Checking for highlighted piece, if flip board option is on then we need to look at the board the opposite way
            // otherwise if flip board is not active then we can always read it 0 - 7, 0 - 
            if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == ChessTypes::Player::PlayerOne && fromHighlightedPiece == &GameBoard[row][col]) || currentTurn == ChessTypes::Player::PlayerTwo && fromHighlightedPiece == &GameBoard[7 - row][7 - col])) || !GameOptions.flipBoardOnNewTurn && (fromHighlightedPiece == &GameBoard[row][col]))
                color = GameOptions.movingPiece_color;

            // Exact same thing as above but excpet now checking for toHighlightedPiece
            else if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == ChessTypes::Player::PlayerOne && toHighlightedPiece == &GameBoard[row][col]) ||
                                                         currentTurn == ChessTypes::Player::PlayerTwo && toHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
                     !GameOptions.flipBoardOnNewTurn && (toHighlightedPiece == &GameBoard[row][col]))
                color = GameOptions.movingToPiece_color;

            if (color == DEFAULT)
                if (currSquare->getOwner() == ChessTypes::Owner::PlayerOne)
                    color = GameOptions.p1_color;
                else
                    color = GameOptions.p2_color;

            setTerminalColor(color);
            WChessPrint(piece.c_str());
            setTerminalColor(DEFAULT);
#ifdef _WIN32
            WChessPrint(" ");
#else 
            WChessPrint(L" ");
#endif
        }
#ifdef _WIN32
        WChessPrint("| "); WChessPrint(std::to_string(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint("\n");
        WChessPrint("\t\t\t  +---+---+---+---+---+---+---+---+\n");
#else 
        WChessPrint(L"| "); WChessPrint(std::to_wstring(CHESS_BOARD_HEIGHT - row).c_str()); WChessPrint(L"\n");
        WChessPrint(L"\t\t\t  +---+---+---+---+---+---+---+---+\n");
#endif
    }
#ifdef _WIN32
    WChessPrint("\t\t\t    a   b   c   d   e   f   g   h\n");
#else 
    WChessPrint(L"\t\t\t    a   b   c   d   e   f   g   h\n");
#endif
    if (!toPrint.empty())
    {
#ifdef _WIN32
        WChessPrint(toPrint.c_str());
#else 
        WChessPrint(std::wstring(toPrint.begin(), toPrint.end()).c_str());
#endif
        toPrint.clear();
    }
}

GameSquare *StandardLocalChessGame::LisolateFromInCheckMoves()
{
    int potential_moves_to_get_out_of_check = kingCanMakeMove ? 1 : 0; // Start at 1 if kingCan make a move
    GameSquare *isolatedPiece = nullptr;
    std::vector<GameSquare *> teamPieces;

    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++)
    {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++)
        {
            GameSquare &gTemp = GameBoard[row][col];

            if (gTemp.getPiece() == ChessTypes::GamePiece::King)
                continue;

            if (static_cast<ChessTypes::Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
        }
    }

    for (auto TeamPiece : teamPieces)
    {

        if (TeamPiece->getPiece() == ChessTypes::GamePiece::Queen)
        {

            // pretend it is a rook temporarily
            TeamPiece->setPiece(ChessTypes::GamePiece::Rook);

            if (LcanDefendKing(teamPieces))
            {
                isolatedPiece = TeamPiece;
                potential_moves_to_get_out_of_check++;
            }
            else
            {
                // pretend it is bishop temporarily
                TeamPiece->setPiece(ChessTypes::GamePiece::Rook);
                if (LcanDefendKing(teamPieces))
                {
                    isolatedPiece = TeamPiece;
                    potential_moves_to_get_out_of_check++;
                }
            }
            pieceCausingKingCheck->setPiece(ChessTypes::GamePiece::Queen);
        }
        else if (LcanDefendKing(teamPieces))
        {
            isolatedPiece = TeamPiece;
            potential_moves_to_get_out_of_check++;
        }

        if (potential_moves_to_get_out_of_check >= 2)
            return nullptr;
    }

    if (kingCanMakeMove)
        return currentTurn == ChessTypes::Player::PlayerOne ? whitePlayerKing : blackPlayerKing;
    return isolatedPiece;
}

// -1 Puts king in harm way
// 0 Invalid move
// 1 Piece taken
// 2 Piece moved
ChessEnums::MakeMoveResult StandardLocalChessGame::LmakeMove(LMove &&move)
{

    if (LreadPossibleMoves(move.getMoveTo()) == ChessEnums::PossibleMovesResult::NOT_FOUND)
        return ChessEnums::MakeMoveResult::InvalidMove;

    // Check if making this move will put their king in check

    bool pieceTake = false;
    bool isKingMove = move.getMoveFrom().getPiece() == ChessTypes::GamePiece::King ? true : false;
    GameSquare saveOldFrom(move.getMoveFrom());
    GameSquare saveOldTo(move.getMoveTo());

    if (move.getMoveTo().getPiece() != ChessTypes::GamePiece::None)
        pieceTake = true;

    // Lets move the piece now, This is also where we would do something different in case of castling since you are not setting the from piece to none / open
    move.getMoveTo().setPiece(move.getMoveFrom().getPiece());
    move.getMoveTo().setOwner(move.getMoveFrom().getOwner());
    move.getMoveFrom().setPiece(ChessTypes::GamePiece::None);
    move.getMoveFrom().setOwner(ChessTypes::Owner::None);

    if (isKingMove)
    {
        if (currentTurn == ChessTypes::Player::PlayerOne)
        {
            whitePlayerKing = &move.getMoveTo();
        }
        else
        {
            blackPlayerKing = &move.getMoveTo();
        }
    }

    if (LkingSafe())
    {
        // Mark this gamesquare that a move has been made on this square
        move.getMoveFrom().setFirstMoveMade();

        if (pieceTake)
            return ChessEnums::MakeMoveResult::PieceTaken;
        else
            return ChessEnums::MakeMoveResult::PieceMoved;
    }

    // Revert move because this made the current turns king not safe
    move.getMoveFrom().setPiece(saveOldFrom.getPiece());
    move.getMoveFrom().setOwner(saveOldFrom.getOwner());
    move.getMoveTo().setPiece(saveOldTo.getPiece());
    move.getMoveTo().setOwner(saveOldTo.getOwner());

    // Revert king pos
    if (isKingMove)
    {
        if (currentTurn == ChessTypes::Player::PlayerOne)
        {
            whitePlayerKing = &move.getMoveFrom();
        }
        else
        {
            blackPlayerKing = &move.getMoveFrom();
        }
    }

    return ChessEnums::MakeMoveResult::KingInDanger;
}

void StandardLocalChessGame::LinitGame()
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

            if (pieceToPut == ChessTypes::GamePiece::King) {
                if (row == (int)ChessTypes::YCoordinate::Eight)
                    blackPlayerKing = &GameBoard[row][col];
                else
                    whitePlayerKing = &GameBoard[row][col];
            }

            if (pieceToPut == ChessTypes::GamePiece::None) // This first move made only applies to pieces with actual pieces there
                GameBoard[row][col].setFirstMoveMade();
        }
    }
}