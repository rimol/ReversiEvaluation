#pragma once
#include <vector>
#include "bitboard.h"

struct Solution {
    int bestScore;
    long long nodeCount;
    // ミリ秒
    double scoreLockTime, wholeTime;
    std::vector<int> bestMoves;

    int NPS() const { return nodeCount / scoreLockTime * 1000; }
};

Solution solve(Bitboard p, Bitboard o);
void ffotest();