#pragma once

#include "../Utils/Point.hpp"
#include "../TypesAndEnums/ChessTypes.hpp"
#include "../Utils/ChessHelperFunctions.hpp" 

class GameSquare {
private:
    bool mfirstMoveOccurred = false; // Applies for castling, pawns, etc.
    Point mPosition;
    ChessTypes::Owner mOwner;
    ChessTypes::GamePiece mPiece;

public:
    // Default constructor should initialize mPosition
    GameSquare() : mPosition({-1, -1}), mOwner(ChessTypes::Owner::None), mPiece(ChessTypes::GamePiece::None) {}

    GameSquare(ChessTypes::Owner owner, ChessTypes::GamePiece piece, Point pos)
        : mPosition(pos), mOwner(owner), mPiece(piece) {}

    GameSquare(const GameSquare& other)
        : mPosition(other.mPosition), mOwner(other.mOwner), mPiece(other.mPiece), mfirstMoveOccurred(other.mfirstMoveOccurred) {}

    // Corrected assignment operator
    GameSquare& operator=(const GameSquare& other) {
        if (this != &other) {
            // We can't assign to mPosition because it's const,
            // so we need to ensure that this assignment operator is only used correctly
            // where mPosition doesn't need to change.
            mOwner = other.mOwner;
            mPiece = other.mPiece;
            mPosition = other.mPosition;
            mfirstMoveOccurred = other.mfirstMoveOccurred;
        }
        return *this;
    }

    bool operator==(const GameSquare& other) const {
        return mfirstMoveOccurred == other.mfirstMoveOccurred &&
               mPosition == other.mPosition &&
               mOwner == other.mOwner &&
               mPiece == other.mPiece;
    }

    void print() const {
        std::wcout << "Pos: {" << mPosition.m_x << ", " << mPosition.m_y << "}, Piece: " 
                   << enumPiece_toString(mPiece) << ", Owner: " 
                   << (mOwner == ChessTypes::Owner::None ? "None" : mOwner == ChessTypes::Owner::PlayerOne ? "Player one" : "Player two");
    }

    void setOwner(ChessTypes::Owner o) { mOwner = o; }
    ChessTypes::Owner getOwner() const { return mOwner; }

    void setPiece(ChessTypes::GamePiece p) { mPiece = p; }
    ChessTypes::GamePiece getPiece() const { return mPiece; }

    Point getPosition() const { return mPosition; }
    void setPosition(Point p) { mPosition = p; }

    void setFirstMoveMade() { mfirstMoveOccurred = true; }
    bool getIfFirstMoveMade() const { return mfirstMoveOccurred; }
};
