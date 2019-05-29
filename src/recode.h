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
    Bitboard p_r[4];
    Bitboard o_r[4];
    int result;

    RecodeEx() {}

    RecodeEx(Recode recode)
        : p_r{ recode.p, rotateRightBy90(recode.p), rotateBy180(recode.p), rotateRightBy90(rotateBy180(recode.p)) },
          o_r{ recode.o, rotateRightBy90(recode.o), rotateBy180(recode.o), rotateRightBy90(rotateBy180(recode.o)) },
          result(recode.result) {}
};