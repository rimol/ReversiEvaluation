#pragma once
#include "bitboard.h"

struct Recode {
    Bitboard board[2];
    Color c;
    // 何手目の盤面か
    int turn;
    // 最終石差(cの石の数-~cの石の数)
    int result;

    Bitboard p() const { return board[c]; }
    Bitboard o() const { return board[~c]; }

    Recode() {}

    Recode(Bitboard b, Bitboard w, Color _c, int _turn)
        : board{b, w}, c(_c), turn(_turn), result() {}
};