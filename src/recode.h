#pragma once
#include "bitboard.h"
// Boardとややこしいが、これでファイルのやりとりをする
struct Recode {
    // 盤面
    Bitboard p;
    Bitboard o;
    // 何手目の盤面か
    int turn;
    // 最終石差(黒-白)
    int result;

    Recode() {}

    Recode(Bitboard _p, Bitboard _o, int _turn)
        : p(_p), o(_o), turn(_turn), result() {}
};