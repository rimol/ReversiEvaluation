#pragma once
#include <vector>
#include "bitboard.h"

struct Solution {
    int bestScore;
    double scoreLockTime, wholeTime;
    std::vector<int> bestMoves;
};

// pからみた最終石差を計算
Solution solve(Bitboard p, Bitboard o, std::vector<int>& ans);
void ffotest();