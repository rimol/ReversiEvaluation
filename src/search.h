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

constexpr int TTSize = 0x200000;

// とりあえず適当に
int inline getIndex(Bitboard p, Bitboard o) {
    return (((p >> 32) * 2 + p * 3 + (o >> 32) * 5 + o * 7) >> 7) & (TTSize - 1);
}