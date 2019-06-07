#pragma once
#include "bitboard.h"

struct Recode {
    Bitboard p, o;
    int result;

    Recode() {}
    Recode(Bitboard p, Bitboard o, int result) : p(p), o(o), result(result) {}
};

// 回転したビットボードもあらかじめ持つ
struct RecodeEx{
    Bitboard playerRotatedBB[4];
    Bitboard opponentRotatedBB[4];
    int result;

    RecodeEx() {}

    RecodeEx(Recode recode)
        : playerRotatedBB{ recode.p, rotateRightBy90(recode.p), rotateBy180(recode.p), rotateRightBy90(rotateBy180(recode.p)) },
          opponentRotatedBB{ recode.o, rotateRightBy90(recode.o), rotateBy180(recode.o), rotateRightBy90(rotateBy180(recode.o)) },
          result(recode.result) {}
};