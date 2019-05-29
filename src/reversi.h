#include <string>
#pragma once

#include <sstream>
#include "bitboard.h"

struct Reversi {
    Color c;
    Bitboard p, o, moves;
    bool isFinished;

    bool move(int sq);
    Color winner() const;
    void print() const;
    int stoneCount() const { return popcount(p | o); }

    Reversi() : c(Black), p(0x0000000810000000ULL), o(0x0000001008000000ULL), moves(getMoves(p, o)), isFinished(false) {}
};

// A1みたいな書式に変換
std::string convertToLegibleSQ(int sq);