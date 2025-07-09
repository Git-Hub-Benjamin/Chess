#pragma once

#include "GameSquare.hpp"

class LMove{

    GameSquare& m_moveFrom;
    GameSquare& m_moveTo;

public:
    LMove(GameSquare& move, GameSquare& to): m_moveFrom(move), m_moveTo(to) {}

    GameSquare& getMoveFrom() const { return m_moveFrom; }
    GameSquare& getMoveTo() const { return m_moveTo; }

    LMove operator=(const LMove& other) {
        if (this != &other) {
            m_moveFrom = other.m_moveFrom;
            m_moveTo = other.m_moveTo;
        }
        return *this;
    }
};