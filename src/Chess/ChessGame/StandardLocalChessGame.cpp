#include "../chess.hpp"
#include "../../client/tui/clientoption.hpp"

#ifdef LEGACY_ARRAY_GAMEBOARD

// True - There is at least one move
// False - No moves from this piece
bool StandardLocalChessGame::LpopulatePossibleMoves(GameSquare& moveFrom) {

    ChessTypes::GamePiece fromPiece = moveFrom.getPiece();
    short possibleMoveCounter = PIECE_MOVE_COUNTS[fromPiece - 1];

    if (fromPiece == PAWN && currentTurn == PlayerTwo)
        fromPiece = OPEN; 

    for(int move_set_count = 0; move_set_count < possibleMoveCounter; move_set_count++){

        // Iterating over entire moveset of a piece to see if it is 

        // 1. on the board
        // 2. unobstructed path
        // 3. piece at square owner is not equal to current turn (except speical moves!)
        // 3. if its a king then making sure nothing can reach that square

        Point pTemp(moveFrom.getPosition().m_x + pieceMovePtrs[fromPiece][move_set_count][0], moveFrom.getPosition().m_y + pieceMovePtrs[fromPiece][move_set_count][1]);
        
        if (!onBoard(pTemp))
            continue;

        Move mTemp(moveFrom, GameBoard[pTemp.m_y][pTemp.m_x]);

        if (!currTurnInCheck) {
            if (mTemp.getMoveTo().getPiece() != KING)
                if (LunobstructedPathCheck(mTemp)) 
                    if (static_cast<ChessTypes::Player>(mTemp.getMoveTo().getOwner()) != currentTurn) {
                        if (mTemp.getMoveFrom().getPiece() == KING) 
                            if (!LkingSafeAfterMove(mTemp.getMoveTo()))
                                continue;
                        possibleMoves.push_back(possibleMoveType(&mTemp.getMoveTo(), 
                        mTemp.getMoveTo().getPiece() == ChessTypes::GamePiece::OPEN ? possibleMoveTypes::POSSIBLE_MOVE_OPEN_SQAURE : possibleMoveTypes::POSSIBLE_MOVE_ENEMY_PIECE));
                    }
        } else {
            // See if making this move would make the kingSafe()
            bool isKingMove = moveFrom.getPiece() == KING;
            GameSquare saveOldFrom(mTemp.getMoveFrom());
            GameSquare saveOldTo(mTemp.getMoveTo());

            mTemp.getMoveTo().setPiece(mTemp.getMoveFrom().getPiece());
            mTemp.getMoveTo().setOwner(mTemp.getMoveFrom().getOwner());
            mTemp.getMoveFrom().setPiece(OPEN);
            mTemp.getMoveFrom().setOwner(NONE);

            // Making king move temporarily
            if (isKingMove) {
                if (currentTurn == PlayerOne) {
                    whitePlayerKing = &mTemp.getMoveTo();
                } else {
                    blackPlayerKing = &mTemp.getMoveTo();
                }
            }

            if(LkingSafe())
                possibleMoves.push_back(possibleMoveType(&mTemp.getMoveTo(),
                isKingMove ? possibleMoveTypes::POSSIBLE_MOVE_KING_IN_DANGER :
                mTemp.getMoveTo().getPiece() == ChessTypes::GamePiece::OPEN ? possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_SQUARE : possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_PIECE));
            
            // Revert
            mTemp.getMoveFrom().setPiece(saveOldFrom.getPiece());
            mTemp.getMoveFrom().setOwner(saveOldFrom.getOwner());
            mTemp.getMoveTo().setPiece(saveOldTo.getPiece());
            mTemp.getMoveTo().setOwner(saveOldTo.getOwner());

            // Revert king pos
            if (isKingMove) {
                if (currentTurn == PlayerOne) {
                    whitePlayerKing = &mTemp.getMoveFrom();
                } else {
                    blackPlayerKing = &mTemp.getMoveFrom();
                }       
            }
        }
    }
    return !possibleMoves.empty();
}

// 0 FREE
// 1 PONE TAKEN
// 2 PTWO TAKEN
enum Owner StandardLocalChessGame::LpiecePresent(Point p){
    enum Owner owner = GameBoard[p.m_y][p.m_x].getOwner();
    if(owner == NONE && GameBoard[p.m_y][p.m_x].getPiece() == OPEN)
        return NONE;
    return owner;    
}

// True valid move
// False invalid move
bool StandardLocalChessGame::LverifyMove(Move& move){
    return validateMoveset(move) && LunobstructedPathCheck(move);
}

bool StandardLocalChessGame::LverifyMove(Move&& move) { 
    return LverifyMove(move);
}

// True - Good, clear path
// False - Something blocking
bool StandardLocalChessGame::LrookClearPath(Move& move){

    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    // Determine if moving along x or y axis
    if(from.m_x == to.m_x){
        // moving along Y axis
        int amount_to_check = std::abs(from.m_y - to.m_y);
        int direction = (from.m_y - to.m_y < 0) ? 1 : -1;

        for(int i = 1; i < amount_to_check; i++){
            struct Point temp = from;
            temp.m_y += (i * direction);
            if(LpiecePresent({temp.m_x, temp.m_y}))
                return false; 
        }
    }else{
        // moving along X axis
        int amount_to_check = std::abs(from.m_x - to.m_x);
        int direction = (from.m_x - to.m_x < 0) ? 1 : -1;

        for(int i = 1; i < amount_to_check; i++){
            struct Point temp = from;
            temp.m_x += (i * direction);
            if(LpiecePresent({temp.m_x, temp.m_y}))
                return false; 
        }
    }
    return true;
}

// True - Good, clear path
// False - Something blocking
bool StandardLocalChessGame::LbishopClearPath(Move& move){
    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();

    int xdir = (from.m_x - to.m_x < 0 ? 1 : -1);
    int ydir = (from.m_y - to.m_y < 0 ? 1 : -1);
    int amount_of_check = std::abs(from.m_x - to.m_x); // i dont think this should matter which one you do
    
    for(int i = 1; i < amount_of_check; i++){
        struct Point temp = from;
        temp.m_x += (i * xdir);
        temp.m_y += (i * ydir);
        if(LpiecePresent({temp.m_x, temp.m_y}))
            return false;
    }
    return true; 
}

// True - Valid move for pawn
// False - Invalid move for pawn
bool StandardLocalChessGame::LpawnMoveCheck(Move& move){

    Point from = move.getMoveFrom().getPosition();
    Point to = move.getMoveTo().getPosition();
    
    // Validates double advance and diagonal only capturing

    // Rules of double advanwcing for pawns
    // 1. Starting Position
    // 2. Unobstructed Path
    // 3. No Capturing

    if(std::abs(from.m_y - to.m_y) == 2){

        // Check rule 2.
        if(std::abs(move.getMoveFrom().getPosition().m_y - move.getMoveTo().getPosition().m_y) == 2)
            if(LpiecePresent({from.m_x, from.m_y + (2 * (currentTurn == PlayerOne ? -1 : 1))}))
                return false;

        // Checking rule 1. and 3.
        if(move.getMoveFrom().getIfFirstMoveMade() || LpiecePresent({to.m_x, to.m_y}))
            return false;

    }else{
        // Now this section if for checking that pawn can only take an oppenents piece if moving diagonally

        if(from.m_x == to.m_x){ 
            // moving forward 1, meaning it cant take pieces
            if(LpiecePresent({to.m_x, to.m_y}))
                return false;
        }else{
            // moving diagonally, meaning it has to take a piece to do this
            if(!LpiecePresent({to.m_x, to.m_y}))
                return false;
        }
    }

    return true; // Meaning this is a valid move
}

// True - All good
// False - Piece in way
bool StandardLocalChessGame::LunobstructedPathCheck(Move& move){
    // Now verify if there is anything in path
    // Dont have to check knight bc it can go through pieces, also king can only go one piece

    switch(move.getMoveFrom().getPiece()){
        case(ROOK):
            return LrookClearPath(move);
        case(QUEEN):
            if(move.getMoveFrom().getPosition().m_x == move.getMoveTo().getPosition().m_x || move.getMoveFrom().getPosition().m_y== move.getMoveTo().getPosition().m_y)
                return LrookClearPath(move);
            else
                return LbishopClearPath(move);
        case(BISHOP):
            return LbishopClearPath(move);
        case(PAWN): 
            return LpawnMoveCheck(move);
        default:
            return true; // Doesnt matter for (KNIGHT, KING & OPEN/NONE)
    }   
}

// True - can defend the king so not checkmate
// False - checkmate!
// if it is a queen causing check, called will temporarily change piece causing check to rook, then change it to bishop then change it back
GameSquare* StandardLocalChessGame::LcanDefendKing(std::vector<GameSquare*>& teamPieces) {
    
    Point pieceCausingCheckPos = pieceCausingKingCheck->getPosition();
    ChessTypes::GamePiece pieceCausingCheckPiece = pieceCausingKingCheck->getPiece();
    Point checkedKingPos = (currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing)->getPosition();

    int xdir = 0;
    int ydir = 0;
    int amount_to_check;

    if (pieceCausingCheckPiece == ROOK) {
            // Moving along Y axis    
        if (pieceCausingCheckPos.m_x == checkedKingPos.m_x)
            ydir = (pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0) ? 1 : -1;
        else// Moving along X axis
            ydir = (pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0) ? 1 : -1;
        // Get amount
        amount_to_check = xdir == 0 ? std::abs(checkedKingPos.m_y - pieceCausingCheckPos.m_y) : std::abs(checkedKingPos.m_x - pieceCausingCheckPos.m_x);
    } else if (pieceCausingCheckPiece == BISHOP) {
        xdir = pieceCausingCheckPos.m_x - checkedKingPos.m_x < 0 ? -1 : 1;
        ydir = pieceCausingCheckPos.m_y - checkedKingPos.m_y < 0 ? -1 : 1;
        amount_to_check = std::abs(pieceCausingCheckPos.m_x - checkedKingPos.m_x); 
    }

    if (pieceCausingCheckPiece == ROOK || pieceCausingCheckPiece == BISHOP) {
        for(int i = 1; i <= amount_to_check; i++){

            // Iterate over teamPieces to see if they can reach the currentSquare
            for(GameSquare* teamPiece: teamPieces){
                if(LverifyMove(Move(*teamPiece, GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x]))) {
                    return &GameBoard[pieceCausingCheckPos.m_y][pieceCausingCheckPos.m_x]; // Some team piece can take or block the enemy piece that is causing the check on the king
                }
            }

            pieceCausingCheckPos.m_x += (1 * xdir);
            pieceCausingCheckPos.m_y += (1 * ydir);
        }
    } else {
        for(auto teamPiece: teamPieces){
            if(LverifyMove(Move(*teamPiece, *pieceCausingKingCheck)))
                return pieceCausingKingCheck; // Some team piece can attack the knight, no gameover
        }
    } 

    return nullptr; // Meaning we could not find a move to make to save the king
}

// If nullptr is passed then it will check for general king saftey against every enemy piece
// If a pointer is passed it will check for saftey against the gamesquare passed
// True - King safe
// False - King NOT safe
bool StandardLocalChessGame::LkingSafe() {

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {

            GameSquare& curr = GameBoard[row][col];

            // If empty square we dont need to check, if current square is owned by 
            // current trun then we dont have to check it 
            if (curr.getOwner() == NONE || static_cast<ChessTypes::Player>(curr.getOwner()) == currentTurn)
                continue;

            if (LverifyMove(Move(curr, *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing)))) {
                pieceCausingKingCheck = &curr;
                return false;
            }
        }
    }

    return true;
}

bool StandardLocalChessGame::LkingSafeAfterMove(GameSquare& to) {
    
    GameSquare& currKing = *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing);
    currKing.setOwner(NONE);
    currKing.setPiece(OPEN);

    bool res = true;

    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++) {

            GameSquare& curr = GameBoard[row][col];

            if (curr.getOwner() == NONE || static_cast<ChessTypes::Player>(curr.getOwner()) == currentTurn)
                continue;

            if (LverifyMove(Move(curr, to))) {
                res = false;
                goto restore_king;
            }
        }
    }
restore_king:
    currKing.setOwner(static_cast<Owner>(currentTurn));
    currKing.setPiece(KING);

    return res;
}

// True Gameover, Current turn loses
// False the king can get out of check
bool StandardLocalChessGame::LcheckMate(){ // Checking everything around the king

    GameSquare& kingToCheckSafteyFor = *(currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing);
    short kingPossibleMoves = KING_POSSIBLE_MOVES; // 8
    std::vector<GameSquare*> teamPieces;
    std::vector<GameSquare*> enemyPieces;

    // Getting enemy and team pieces
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            GameSquare& gTemp = GameBoard[row][col];
            if(gTemp.getOwner() == NONE)
                continue;

            if(gTemp.getPiece() == KING)
                continue;

            else if(static_cast<ChessTypes::Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
            else
                enemyPieces.push_back(&gTemp);
        }
    }
    
    // Check each square around the King, if its not on the board SKIP it
    // If its taken by a teammate then SKIP it, bc the king cant take own piece
    //! If its taken by an oppnent then we need to check if its a pawn OR knight

    // Since we are checking these pieces assuming the king is moving there we need to also assume the king has moved
    kingToCheckSafteyFor.setOwner(NONE);
    kingToCheckSafteyFor.setPiece(OPEN);

    bool res = true;

    for(int move_set_count = 0; move_set_count < kingPossibleMoves; move_set_count++){

        Point currKingPosAroundKing(kingToCheckSafteyFor.getPosition().m_x + pieceMovePtrs[KING][move_set_count][0], kingToCheckSafteyFor.getPosition().m_y + pieceMovePtrs[KING][move_set_count][1]);

        if(!onBoard(currKingPosAroundKing)) 
            continue; 

        // ! You would need to check here if the king can castle to get out of danger
        if(static_cast<ChessTypes::Player>(GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x].getOwner()) == currentTurn)
            continue; 

        GameSquare& currSquareAroundKingCheck = GameBoard[currKingPosAroundKing.m_y][currKingPosAroundKing.m_x];
        bool ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = false;

        for(auto enemy: enemyPieces){

            if(LverifyMove(Move(*enemy, currSquareAroundKingCheck))) {
                ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE = true;
                break; // No point to keep checking, we know this square is NOT safe
            }
        }

        if (!ENEMY_CAN_ATTACK_KING_SURROUNDING_SQUARE) {
            res = false;
            goto restore_king;
        }
    }

restore_king:

    kingToCheckSafteyFor.setOwner(static_cast<Owner>(currentTurn));
    kingToCheckSafteyFor.setPiece(KING);

    if(!res)
        return false;

    kingCanMakeMove = false;

    // Check if any of the teampieces can defend the king by either taking the piece causing check OR blocking its path
    
    if (pieceCausingKingCheck->getPiece() == QUEEN) {
        
        // pretend it is a rook temporarily
        pieceCausingKingCheck->setPiece(ROOK);

        if (LcanDefendKing(teamPieces)) {
            pieceCausingKingCheck->setPiece(QUEEN);
            return false;
        }

        // pretend it is bishop temporarily
        pieceCausingKingCheck->setPiece(ROOK);

        bool res = LcanDefendKing(teamPieces);

        pieceCausingKingCheck->setPiece(QUEEN);

        return res;
    }

    return LcanDefendKing(teamPieces) == nullptr; // If any of the team pieces can defend the king then this will result in NO checkmate, otherwise checkmate
}

GameSquare& StandardLocalChessGame::LconvertMove(std::wstring move, ChessTypes::Player sideToConvert){
    // convert letter to number (a = 0, b = 1 etc)
    // convert char number to number ('0' = 0 etc)
    // minus 8 is important since (0,0) is flipped since 8 starts at top

    int row = 8 - (move[1] - 48);
    int col = move[0] - 97;

    if ((GameConnectivity == ONLINE_CONNECTIVITY && sideToConvert == PlayerTwo) ||
        (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && sideToConvert == PlayerTwo))
            return GameBoard[LreflectAxis(row)][LreflectAxis(col)];
        
    return GameBoard[row][col];

}

// No Piece Present - 0
// This Piece Does not belong to you - 1
// Cannot take your own piece - 2
// Valid - 3
validateGameSquare::validateGameSquare StandardLocalChessGame::LvalidateGameSquare(GameSquare& square, enum getMoveType getMoveType){
    if (getMoveType == getMoveType::GET_FROM) {

        if(square.getOwner() == NONE)
            return validateGameSquare::NO_PIECE;

        if(static_cast<ChessTypes::Player>(square.getOwner()) != currentTurn)
            return validateGameSquare::PIECE_NOT_YOURS;

    } else {
        if(static_cast<ChessTypes::Player>(square.getOwner()) == currentTurn)
            return validateGameSquare::CANNOT_TAKE_OWN;
    }
    
    return validateGameSquare::VALID;
}

// True found matching move with possibleMoves
// False not found
possibleMoveTypes StandardLocalChessGame::LreadPossibleMoves(GameSquare& to) {
    for(possibleMoveType& possibleMove: possibleMoves)
        if(to == *possibleMove.m_boardSquare)
            return possibleMove.possibleMoveTypeSelector;
    return possibleMoveTypes::NOT_FOUND;
}

void StandardLocalChessGame::LstartGame(){

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

        // Reset check
        currTurnInCheck = false;
        kingCanMakeMove = true;

        if (!LkingSafe()) {
            if (LcheckMate()) {
                printBoard(currentTurn);
                GameOver = true;
                std::wcout << "GameOver!!" << std::endl;
                break;
            }
            else {
                currTurnInCheck = true;
            }
        }

        while (true) {

            possibleMoves.clear(); // Clear the possibleMoves vec
            fromHighlightedPiece = nullptr; // Make sure this is not set
            toHighlightedPiece = nullptr; // Make sure this is not set
            inputBuffer.clear(); // Make sure this is empty

            std::wstring moveFrom;
            std::wstring moveTo;
            GameSquare *oneMoveFromCheck = nullptr;

            if (currTurnInCheck)
                oneMoveFromCheck = LisolateFromInCheckMoves();

            if (oneMoveFromCheck == nullptr) {
                printBoard(currentTurn);

                int res = getMove(getMoveType::GET_FROM);

                if (res == getMove::QUIT) { // Quit
                    GameOver = true; 
                    break;
                } else if (res == getMove::TIMER_RAN_OUT) { // Timer ran out, end game, win for other player
                    std::wcout << "Timer ran out..." << std::endl;
                    GameOver = true;
                    break;
                } else if (GameOptions.boardHistory && (res == getMove::UNDO || res == getMove::REDO)) { // Undoing turn
                    if (res == getMove::UNDO)
                        std::wcout << "Undoing turn..." << std::endl;
                    else
                        std::wcout << "Redoing turn..." << std::endl;
                    isLoadingState = true;
                    break;
                } 

                moveFrom = inputBuffer;
                inputBuffer.clear();
    
                res = static_cast<int>(LvalidateGameSquare(LconvertMove(moveFrom, currentTurn), getMoveType::GET_FROM));
                if (res == validateGameSquare::NO_PIECE) {
                    toPrint = L"No piece present.";
                    continue;
                }
                else if (res == validateGameSquare::PIECE_NOT_YOURS) {
                    toPrint = L"This piece does not belong to you.";
                    continue;
                }

                // We need to populate possible moves regardless of whether or not we have move highlighing,
                // if dynamic move highlighting is enabled this is already done in getMove
                if (!GameOptions.dynamicMoveHighlighting) {
                    if (!LpopulatePossibleMoves(LconvertMove(moveFrom, currentTurn))) {
                        if (currTurnInCheck)
                            toPrint = L"You need to protect your king.";
                        else
                            toPrint = L"No moves with that piece.";
                        continue;
                    }
                    if (GameOptions.moveHighlighting)
                        printBoardWithMoves(currentTurn);
                }
            } else {
                if (!LpopulatePossibleMoves(*oneMoveFromCheck)) 
                    //! FATAL ERROR THIS SHOULD NOT HAPPEN
                printBoardWithMoves(currentTurn);
            }

            // --------------------------------- //

            int res = getMove(getMoveType::GET_TO);

            if (res == getMove::QUIT) { // Quit
                GameOver = true; 
                break;
            } else if (res == getMove::TIMER_RAN_OUT) { // Timer ran out, end game, win for other player
                std::wcout << "Timer ran out..." << std::endl;
                GameOver = true;
                break;
            } else if (res == getMove::UNDO || res == getMove::REDO) { // Undoing turn
                if (res == getMove::UNDO)
                    std::wcout << "Undoing turn..." << std::endl;
                else
                    std::wcout << "Redoing turn..." << std::endl;
                isLoadingState = true;
                break;
            } else if (res == getMove::CHOOSE_MOVE_AGAIN)
                continue;

            res = LvalidateGameSquare(LconvertMove(moveTo, currentTurn), getMoveType::GET_TO);
            if (res == validateGameSquare::CANNOT_TAKE_OWN) {
                toPrint = L"Cannot take your own piece.";
                continue;
            }

            moveTo = inputBuffer;

            // ---------------------------------- //

            res = LmakeMove(Move(oneMoveFromCheck == nullptr ? LconvertMove(moveFrom, currentTurn) : *oneMoveFromCheck, LconvertMove(moveTo, currentTurn)));
            if (res == makeMove::KING_IN_HARM)
                toPrint = L"This puts your king in danger!";
            else if (res == makeMove::INVALID_MOVE)
                toPrint = L"Invalid move.";
            else if (res == makeMove::PIECE_TAKEN)
                toPrint = L"Piece taken.";
            else
                toPrint = L"Piece moved.";

            if (res < 1)
                continue; // Redo turn

            break; // Next turn
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

//* may just override in the future, but having L "giving itself its own function" is fine for now
void StandardLocalChessGame::LprintBoard(ChessTypes::Player playerSideToPrint){
    if (GameOptions.clearScreenOnBoardPrint)
        system("clear");
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - row << " ";
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){
            std::wcout << "| ";
            wchar_t piece;


            if ((GameConnectivity == ONLINE_CONNECTIVITY && playerSideToPrint == PlayerTwo) || (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && playerSideToPrint == PlayerTwo)) {
                if(GameBoard[7 - row][7 - col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[7 - row][7 - col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }
            } else {
                if(GameBoard[row][col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[row][col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }
            }
            
            // if (GameOptions.flipBoardOnNewTurn && currentTurn == PlayerTwo) {
            //     if(GameBoard[7 - row][7 - col].getOwner() == NONE)
            //         piece = ' ';
            //     else if(GameBoard[7 - row][7 - col].getOwner() == PONE){
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
            //         set_terminal_color(GameOptions.p1_color);
            //     }else{
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
            //         set_terminal_color(GameOptions.p2_color);
            //     }
            // } else {
            //     if(GameBoard[row][col].getOwner() == NONE)
            //         piece = ' ';
            //     else if(GameBoard[row][col].getOwner() == PONE){
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[row][col].getPiece()];
            //         set_terminal_color(GameOptions.p1_color);
            //     }else{
            //         piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[row][col].getPiece()];
            //         set_terminal_color(GameOptions.p2_color);
            //     }
            // }
            
            std::wcout << piece;
            set_terminal_color(DEFAULT);
            std::wcout << " ";
            
        }
        std::wcout << "| " << CHESS_BOARD_HEIGHT - row << std::endl;
        std::wcout << "\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t    a   b   c   d   e   f   g   h\n";

    if (!toPrint.empty()) {
        std::wcout << toPrint << std::endl;
        toPrint.clear();
    }
}

int StandardLocalChessGame::LreflectAxis(int val) {
    switch (val) {
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


void StandardChessGame::LprintBoardWithMoves(ChessTypes::Player playerSideToPrint) {
    if(GameOptions.clearScreenOnBoardPrint)
        system("clear");
    std::wcout << "\n\n\n\t\t\t    a   b   c   d   e   f   g   h\n" << "\t\t\t  +---+---+---+---+---+---+---+---+\n";
    for(int row = 0; row < CHESS_BOARD_HEIGHT; row++){
        std::wcout << "\t\t\t" << CHESS_BOARD_HEIGHT - row << " ";
        for(int col = 0; col < CHESS_BOARD_WIDTH; col++){

            std::wcout << "| ";
            wchar_t piece;
            possibleMoveTypes possibleMoveTypeSelector;

            if ((GameConnectivity == ONLINE_CONNECTIVITY && playerSideToPrint == PlayerTwo) || (GameConnectivity == LOCAL_CONNECTIVITY && GameOptions.flipBoardOnNewTurn && playerSideToPrint == PlayerTwo)) {
                  if(GameBoard[7 - row][7 - col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[7 - row][7 - col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[7 - row][7 - col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }

                // checking if the current square can be acctacked by piece
                possibleMoveTypeSelector = readPossibleMoves(GameBoard[7 - row][7 - col]);
                
            } else {
                if(GameBoard[row][col].getOwner() == NONE)
                    piece = ' ';
                else if(GameBoard[row][col].getOwner() == PONE){
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.whitePlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p1_color);
                }else{
                    piece = TEXT_PIECE_ART_COLLECTION[GameOptions.blackPlayerArtSelector][GameBoard[row][col].getPiece()];
                    set_terminal_color(GameOptions.p2_color);
                }

                // checking if the current square can be acctacked by piece
                possibleMoveTypeSelector = readPossibleMoves(GameBoard[row][col]);
            }

            if(possibleMoveTypeSelector != possibleMoveTypes::NOT_FOUND){
                if(piece == ' ')
                    piece = 'X';
                WRITE_COLOR color;
                switch(possibleMoveTypeSelector) {
                    case possibleMoveTypes::POSSIBLE_MOVE_ENEMY_PIECE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_OPEN_SQAURE:
                        color = GameOptions.possibleMove_color;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_PIECE:
                        color = WRITE_COLOR::BRIGHT_MAGENTA;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_PROTECT_KING_SQUARE:
                        color = WRITE_COLOR::BRIGHT_YELLOW;
                        break;
                    case possibleMoveTypes::POSSIBLE_MOVE_KING_IN_DANGER:
                        color = WRITE_COLOR::YELLOW;
                        break;
                    default:
                        color = WRITE_COLOR::BLACK;
                        break;
                }
                set_terminal_color(color);
            }

            // Checking for highlighted piece, if flip board option is on then we need to look at the board the opposite way
            // otherwise if flip board is not active then we can always read it 0 - 7, 0 - 7
            if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == PlayerOne && fromHighlightedPiece == &GameBoard[row][col]) ||
                currentTurn == PlayerTwo && fromHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
                !GameOptions.flipBoardOnNewTurn && (fromHighlightedPiece == &GameBoard[row][col]))
                set_terminal_color(GameOptions.movingPiece_color);

            // Exact same thing as above but excpet now checking for toHighlightedPiece
            else if ((GameOptions.flipBoardOnNewTurn && ((currentTurn == PlayerOne && toHighlightedPiece == &GameBoard[row][col]) ||
                currentTurn == PlayerTwo && toHighlightedPiece == &GameBoard[7 - row][7 - col])) ||
                !GameOptions.flipBoardOnNewTurn && (toHighlightedPiece == &GameBoard[row][col]))
                set_terminal_color(GameOptions.movingToPiece_color);
            
            std::wcout << piece;    
            set_terminal_color(DEFAULT);
            std::wcout << " ";
              
        }
        std::wcout << "| " << CHESS_BOARD_HEIGHT - row << std::endl;
        std::wcout << "\t\t\t  +---+---+---+---+---+---+---+---+" << std::endl;
    }
    std::wcout << "\t\t\t    a   b   c   d   e   f   g   h\n";
    if (!toPrint.empty()) {
        std::wcout << toPrint << std::endl;
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

            if (gTemp.getPiece() == KING)
                continue;

            if (static_cast<ChessTypes::Player>(gTemp.getOwner()) == currentTurn)
                teamPieces.push_back(&gTemp);
        }
    }

    for (auto TeamPiece : teamPieces)
    {

        if (TeamPiece->getPiece() == QUEEN)
        {

            // pretend it is a rook temporarily
            TeamPiece->setPiece(ROOK);

            if (LcanDefendKing(teamPieces))
            {
                isolatedPiece = TeamPiece;
                potential_moves_to_get_out_of_check++;
            }
            else
            {
                // pretend it is bishop temporarily
                TeamPiece->setPiece(ROOK);
                if (LcanDefendKing(teamPieces))
                {
                    isolatedPiece = TeamPiece;
                    potential_moves_to_get_out_of_check++;
                }
            }
            pieceCausingKingCheck->setPiece(QUEEN);
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
        return currentTurn == PlayerOne ? whitePlayerKing : blackPlayerKing;
    return isolatedPiece;
}

// -1 Puts king in harm way
// 0 Invalid move
// 1 Piece taken
// 2 Piece moved
makeMove::makeMove StandardLocalChessGame::LmakeMove(Move&& move){
    
    if(!LreadPossibleMoves(move.getMoveTo())) 
        return makeMove::INVALID_MOVE;

    // Check if making this move will put their king in check

    bool pieceTake = false;
    bool isKingMove = move.getMoveFrom().getPiece() == KING ? true : false;
    GameSquare saveOldFrom(move.getMoveFrom());
    GameSquare saveOldTo(move.getMoveTo());

    if(move.getMoveTo().getPiece() != OPEN)
        pieceTake = true;

    // Lets move the piece now, This is also where we would do something different in case of castling since you are not setting the from piece to none / open
    move.getMoveTo().setPiece(move.getMoveFrom().getPiece());
    move.getMoveTo().setOwner(move.getMoveFrom().getOwner());
    move.getMoveFrom().setPiece(OPEN);
    move.getMoveFrom().setOwner(NONE);

    if (isKingMove) {
        if (currentTurn == PlayerOne) {
            whitePlayerKing = &move.getMoveTo();
        } else {
            blackPlayerKing = &move.getMoveTo();
        }
    }

    if(LkingSafe()){
        // Mark this gamesquare that a move has been made on this square
        move.getMoveFrom().setFirstMoveMade();

        if(pieceTake)
            return makeMove::PIECE_TAKEN;
        else 
            return makeMove::PIECE_MOVED;
    }

    // Revert move because this made the current turns king not safe
    move.getMoveFrom().setPiece(saveOldFrom.getPiece());
    move.getMoveFrom().setOwner(saveOldFrom.getOwner());
    move.getMoveTo().setPiece(saveOldTo.getPiece());
    move.getMoveTo().setOwner(saveOldTo.getOwner());

    // Revert king pos
    if (isKingMove) {
        if (currentTurn == PlayerOne) {
            whitePlayerKing = &move.getMoveFrom();
        } else {
            blackPlayerKing = &move.getMoveFrom();
        }
    }

    return makeMove::KING_IN_HARM;
}

void StandardLocalChessGame::LinitGame() {
    for (int row = 0; row < CHESS_BOARD_HEIGHT; row++) {
        for (int col = 0; col < CHESS_BOARD_WIDTH; col++) {
            ChessTypes::GamePiece pieceToPut = OPEN;
            Owner playerOwnerToPut = NONE;

            if (row < Y2) playerOwnerToPut = PTWO;
            if (row > Y5) playerOwnerToPut = PONE;

            if (row == Y0 || row == Y7) {
                switch (col) {
                    case X0:
                    case X7:
                        pieceToPut = ROOK;
                        break;
                    case X1:
                    case X6:
                        pieceToPut = KNIGHT;
                        break;
                    case X2:
                    case X5:
                        pieceToPut = BISHOP;
                        break;
                    case X3:
                        pieceToPut = KING;
                        break;
                    case X4:
                        pieceToPut = QUEEN;
                        break;
                }
            }

            if (row == Y1 || row == Y6) 
                pieceToPut = PAWN;

            GameBoard[row][col] = GameSquare(playerOwnerToPut, pieceToPut, Point(col, row));

            // if (pieceToPut == KING) {
            //     if (row == Y0)
            //         blackPlayerKing = &GameBoard[row][col];
            //     else
            //         whitePlayerKing = &GameBoard[row][col];
            // }

            if (pieceToPut == OPEN) // This first move made only applies to pieces with actual pieces there
                GameBoard[row][col].setFirstMoveMade();
        }
    }
}

#endif

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
        initGame();
}


void StandardLocalChessGame::loadGameState(StandardChessGameHistoryState &state)
{
#ifdef LEGACY_ARRAY_GAMEBOARD
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


#else
// Bit board implementation not implemented yet...
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
        ch+=2;

    ChessEnums::GameOptionResult res;

    switch (ch) {
        case '1':
            change_player_color_option();
            res = ChessEnums::GameOptionResult::CONTINUE;
            break;
        case '2':
            std::wcout << L"Not implemented." << std::endl;
            return ChessEnums::GameOptionResult::CONTINUE; // NOT IMPLEMENTED
        case '3': // Undo
            if (!undoTurn.empty()) {
                StandardChessGameHistoryState state = undoTurn.top();
                loadGameState(state);
                redoTurn.push(state);
                return ChessEnums::GameOptionResult::CONTINUE;
            } else 
                std::wcout << "No moves to undo..." << std::endl;
            return ChessEnums::GameOptionResult::CONTINUE;
        case '4': // Redo
            if (!redoTurn.empty()) {
                StandardChessGameHistoryState state = redoTurn.top();
                loadGameState(state);
                undoTurn.push(state);
                return ChessEnums::GameOptionResult::CONTINUE;
            }else
                std::wcout << "No moves to redo..." << std::endl;
            return ChessEnums::GameOptionResult::CONTINUE;
        case '5':
            return ChessEnums::GameOptionResult::CONTINUE; // NOT IMPLEMENTED
        case '6':
        case 'q':
            return ChessEnums::GameOptionResult::QUIT;
        default:
            return ChessEnums::GameOptionResult::INVALID;
    }

    if (ch == '1' || ch == '2') {
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
ChessEnums::GetMoveResult StandardLocalChessGame::getMove(ChessTypes::GetMoveType getMoveType) {

    // Needs non canonical mode
    if (isClock || GameOptions.dynamicMoveHighlighting) {
#ifdef __linux__
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
            clockThread = std::thread(&StandardLocalChessGame::currTurnChessClock, this, std::ref(stopTimerDisplay), pipe_fd[1], std::wstring(getMoveType == getMoveType::GET_FROM ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ") : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: "));
        else 
            std::wcout << (getMoveType == getMoveType::GET_FROM ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ") : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: ") << std::flush;

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
                                            clockThread = std::thread(&StandardLocalChessGame::currTurnChessClock, this, std::ref(stopTimerDisplay), pipe_fd[1], std::wstring(getMoveType == getMoveType::GET_FROM ? (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ") : !currTurnInCheck ? playerToString(currentTurn) + L", To: "
                                                                                                                                                                                                                                                                                                                                        : playerToString(currentTurn) + L", You're in check! To: "));
                                        }

                                        std::wcout << L"\033[2K\r";
                                        std::wcout << (getMoveType == getMoveType::GET_FROM ? 
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

                                if (inputBuffer == L"re" && getMoveType == getMoveType::GET_TO)
                                    return getMove::CHOOSE_MOVE_AGAIN;

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

                //! THIS NEEDS TO BE UPDATED FOR BITBOARD IMPLEMENTATION
                if (GameOptions.dynamicMoveHighlighting) {
                    if (getMoveType == getMoveType::GET_FROM) {
                        if (sanitizeGetMove(inputBuffer) == 1 && LvalidateGameSquare(LconvertMove(inputBuffer, currentTurn), getMoveType::GET_FROM) == validateGameSquare::VALID) {
                            // This move is valid (on the board) and belongs to the player
                            fromHighlightedPiece = &LconvertMove(inputBuffer, currentTurn);
                            if (LpopulatePossibleMoves(LconvertMove(inputBuffer, currentTurn))) {
                                    printBoardWithMoves(currentTurn);
                                validMoveForMoveHighlightingPrintBoard = true;
                            } else
                                possibleMoves.clear(); // i dont think we need this?
                        } else {
                            if (validMoveForMoveHighlightingPrintBoard) {
                                fromHighlightedPiece = nullptr;
                                validMoveForMoveHighlightingPrintBoard = false;
                                possibleMoves.clear();
                                printBoard(currentTurn);
                            }
                        }
                    } else {
                        if (sanitizeGetMove(inputBuffer) == 1 && LvalidateGameSquare(LconvertMove(inputBuffer, currentTurn), getMoveType::GET_TO) == validateGameSquare::VALID) {
                            toHighlightedPiece = &LconvertMove(inputBuffer, currentTurn);
                            validMoveForMoveHighlightingPrintBoard = true;
                            printBoardWithMoves(currentTurn);
                        } else {
                            if (validMoveForMoveHighlightingPrintBoard) {
                                toHighlightedPiece = nullptr;
                                validMoveForMoveHighlightingPrintBoard = true;
                                printBoardWithMoves(currentTurn);
                            }
                        }
                    }
                }

                // If there is no clock then we need to print the new buffer to the screen

                if (!isClock && !inOptionMenu) { // Clear line and return cursor to the beginning
                    std::wcout << L"\033[2K\r";
                    std::wcout << (getMoveType == getMoveType::GET_FROM ? 
                        (!currTurnInCheck ? playerToString(currentTurn) + L", Move: " : playerToString(currentTurn) + L", You're in check! Move: ")
                        : !currTurnInCheck ? playerToString(currentTurn) + L", To: " : playerToString(currentTurn) + L", You're in check! To: ") 
                    << inputBuffer << std::flush;
                } else if (inOptionMenu) {
                    std::wcout << L"\033[2K\r";
                    std::wcout << "--> " << inputBuffer << std::flush;
                }
            }
        }
#elif _WIN32
    std::wcout << "Clock and move highlighting not yet supported for windows...\n";
    exit(EXIT_SUCCESS);
#endif 
    } else {
        while (true) {
            if (getMoveType == ChessTypes::GetMoveType::From) {
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
                case ChessEnums::SanitizeGetMoveResult::INVALID: // Invalid
                    continue;
                case ChessEnums::SanitizeGetMoveResult::OPTIONS:
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
                        case ChessEnums::GameOptionResult::INVALID: // Invalid input
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
                default:
                    break;
                }
            break; // Continue to next getmove iteration
        }
            return ChessEnums::GetMoveResult::VALID;
    }
}

void StandardLocalChessGame::currTurnChessClock(bool &stop_display, int writePipeFd, const std::wstring &out)
{
    int &count = *(currentTurn == ChessTypes::Player::PlayerOne ? gameClock.getWhiteTimeAddr() : gameClock.getBlackTimeAddr());
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

    #ifdef __linux__
    if (count <= 0)
        write(writePipeFd, "1", 1); // telling main thread that the timer ran out of time
    #endif
}


void StandardLocalChessGame::startGame() {
#ifdef LEGACY_ARRAY_GAMEBAORD
    LstartGame(); return;
#else
    std::wcout << "BITBOARD NOT IMPLEMENTED YET..." << std::endl;
    exit(EXIT_SUCCESS);
#endif
}