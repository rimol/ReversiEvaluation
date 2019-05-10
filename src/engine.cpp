#include "eval.h"
#include "engine.h"

double alphabeta(Bitboard p, Bitboard o, double min, double max, bool passed, int depth) {
    if (depth == 0) return evaluate(p, o);

    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL) return passed ? evaluate(p, o) : -alphabeta(o, p, -max, -min, true, depth);
    
    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        double score = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -max, -min, false, depth - 1);
        if (score >= max) return score;

        min = std::max(score, min);
        moves ^= sqbit;
    }

    return min;
}

int chooseBestMove(Bitboard p, Bitboard o, int depth) {
    double bestScore = -10e9-7;
    int sq = -1;

    Bitboard moves = getMoves(p, o);
    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        double score = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -10e9-7, -bestScore, false, depth - 1);
        if (score > bestScore) {
            bestScore = score;
            sq = tzcnt(sqbit);
        }
        moves ^= sqbit;
    }

    return sq;
}

int chooseRandomMove(Bitboard p, Bitboard o, std::mt19937& mt) {
    Bitboard moves = getMoves(p, o);
    int mobility = popcount(moves);
    int chosen = mt() % mobility;

    for (int i = 0; i < chosen; ++i) {
        moves &= moves - 1ULL;
    }

    return tzcnt(moves);
}