#pragma once

#include "bitboard.h"

template <typename T>
struct SearchedPosition {
    T upper;
    T lower;

    Bitboard p;
    Bitboard o;

    bool correspondsTo(Bitboard _p, Bitboard _o) {
        return _p == p && _o == o;
    }
};

template <typename T>
struct CandidateMove {
    Bitboard nextP;
    Bitboard nextO;
    T evalValue;

    // ソート用
    bool operator<(const CandidateMove &cm) const {
        return evalValue < cm.evalValue;
    }

    bool operator>(const CandidateMove &cm) const {
        return evalValue > cm.evalValue;
    }
};

constexpr int TTSize = 0x80000;

// とりあえず適当に
int inline getIndex(Bitboard p, Bitboard o) {
    return (((p >> 32) * 2 + p * 3 + (o >> 32) * 5 + o * 7) >> 7) & (TTSize - 1);
}