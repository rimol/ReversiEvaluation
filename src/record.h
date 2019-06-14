#pragma once
#include "bitboard.h"

struct Record {
    Bitboard p, o;
    int result;

    Record() {}
    Record(Bitboard p, Bitboard o, int result) : p(p), o(o), result(result) {}
};

// 回転したビットボードもあらかじめ持つ
struct RecordEx{
    Bitboard playerRotatedBB[4];
    Bitboard opponentRotatedBB[4];
    int result;

    RecordEx() {}

    RecordEx(Record record)
        : playerRotatedBB{ record.p, rotateRightBy90(record.p), rotateBy180(record.p), rotateRightBy90(rotateBy180(record.p)) },
          opponentRotatedBB{ record.o, rotateRightBy90(record.o), rotateBy180(record.o), rotateRightBy90(rotateBy180(record.o)) },
          result(record.result) {}
};