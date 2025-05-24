#pragma once

#include <iostream>
#include "../../Util/Terminal/Terminal.hpp"

struct Point{
    int m_x;
    int m_y;
    
    Point() : m_x(-1), m_y(-1) {}
    Point(int x, int y) : m_x(x), m_y(y) {}

    Point operator+(const Point& other) const {
        return Point(this->m_x + other.m_x, this->m_y + other.m_y); 
    }

    bool operator==(const Point other){
        return this->m_x == other.m_x && this->m_y == other.m_y;
    }

    bool operator==(const Point& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }

    void print() { WChessPrint("(X: "); WChessPrint(std::to_string(m_x).c_str()); WChessPrint(", Y: "); WChessPrint(std::to_string(m_y).c_str()); WChessPrint(")\n"); }
};
