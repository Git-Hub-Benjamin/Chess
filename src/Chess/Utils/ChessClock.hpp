#pragma once

class ChessClock
{
    int mWhitePlayerTime = -1;
    int mBlackPlayerTime = -1;

public:
    ChessClock() {}
    ChessClock(ChessClock &copy) : mWhitePlayerTime(copy.mWhitePlayerTime), mBlackPlayerTime(copy.mBlackPlayerTime) {}
    void initTime(int white, int black)
    {
        mWhitePlayerTime = white;
        mBlackPlayerTime = black;
    }
    ChessClock &operator=(const ChessClock &other)
    {
        if (this != &other)
        {
            mWhitePlayerTime = other.mWhitePlayerTime;
            mBlackPlayerTime = other.mBlackPlayerTime;
        }
        return *this;
    }

    int getWhiteSeconds() { return mWhitePlayerTime; }
    int getBlackSeconds() { return mBlackPlayerTime; }
    int *getWhiteTimeAddr() { return &mWhitePlayerTime; }
    int *getBlackTimeAddr() { return &mBlackPlayerTime; }
};