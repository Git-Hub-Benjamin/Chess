#pragma once

#include <iostream>

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

    void print() { std::wcout << "(X: " << m_x << ", Y: " << m_y << ")"; }
};
