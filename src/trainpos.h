#pragma once
#include "bitboard.h"

struct TrainingPosition {
    Bitboard p, o;
    int result;

    TrainingPosition() {}
    TrainingPosition(Bitboard p, Bitboard o) : p(p), o(o) {}
    TrainingPosition(Bitboard p, Bitboard o, int result) : p(p), o(o), result(result) {}
};

// 回転したビットボードもあらかじめ持つ
struct TrainingPositionEx {
    Bitboard rotatedP[8];
    Bitboard rotatedO[8];
    int result;

    TrainingPositionEx() {}

    TrainingPositionEx(TrainingPosition pos) : result(pos.result) {
        rotateAndFlipBB(pos.p, rotatedP);
        rotateAndFlipBB(pos.o, rotatedO);
    }
};