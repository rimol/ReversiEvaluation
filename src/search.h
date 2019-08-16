#include "bitboard.h"

struct CandidateMove {
    Bitboard nextP;
    Bitboard nextO;
    int evalValue;

    // ソート用
    bool operator<(const CandidateMove &cm) const {
        return evalValue < cm.evalValue;
    }

    bool operator>(const CandidateMove &cm) const {
        return evalValue > cm.evalValue;
    }
};

struct SearchedPosition {
    // ミニマックス値の存在範囲
    int upper;
    int lower;

    Bitboard p;
    Bitboard o;

    bool correspondsTo(Bitboard _p, Bitboard _o) {
        return _p == p && _o == o;
    }
};

constexpr int TTSize = 0x200000;

// とりあえず適当に
int inline getIndex(Bitboard p, Bitboard o) {
    return ((p >> 32) * 2 + p * 3 + (o >> 32) * 5 + o * 7) >> 7 & (TTSize - 1);
}