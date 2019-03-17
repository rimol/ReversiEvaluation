#pragma once
#include "board.h"
// Boardとややこしいが、これでファイルのやりとりをする
struct Recode {
    // 盤面
    Bitboard bits[2];
    // 何手目の盤面か
    int turn;
    // 最終石差(黒-白)
    int result;

    Recode() {}

    Recode(Board b, int _turn, int _result)
        : bits({b.bits[Black], b.bits[White]})
        , turn(_turn), result(_result) {}
};