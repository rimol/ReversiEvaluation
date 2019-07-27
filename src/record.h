#pragma once
#include "bitboard.h"

struct Record {
    Bitboard p, o;
    int result;

    Record() {}
    Record(Bitboard p, Bitboard o) : p(p), o(o) {}
    Record(Bitboard p, Bitboard o, int result) : p(p), o(o), result(result) {}
};

// 回転したビットボードもあらかじめ持つ
struct RecordEx {
    Bitboard playerRotatedBB[8];
    Bitboard opponentRotatedBB[8];
    int result;

    RecordEx() {}

    RecordEx(Record record) : result(record.result) {
        rotateAndFlipBB(record.p, playerRotatedBB);
        rotateAndFlipBB(record.o, opponentRotatedBB);
    }
};