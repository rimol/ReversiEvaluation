#include "engine.h"
#include "eval.h"

double alphabeta(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    if (depth == 0)
        return evaluate_classic(p, o);

    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL)
        return passed ? evaluate(p, o) : -alphabeta(o, p, -beta, -alpha, true, depth);

    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        double score = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -beta, -alpha, false, depth - 1);
        if (score >= beta)
            return score;

        alpha = std::max(score, alpha);
        moves ^= sqbit;
    }

    return alpha;
}

std::vector<MoveWithScore> evalAllMoves(Bitboard p, Bitboard o, int depth) {
    std::vector<MoveWithScore> movesWithScore;
    Bitboard moves = getMoves(p, o);

    constexpr double inf = 10e9;

    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);

        int sq = tzcnt(sqbit);
        double score = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -inf, inf, false, depth - 1);
        movesWithScore.push_back({sq, score});
    }

    return movesWithScore;
}

int chooseBestMove(Bitboard p, Bitboard o, int depth) {
    double bestScore = -10e9 - 7;
    int sq = -1;

    Bitboard moves = getMoves(p, o);
    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        double score = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -10e9 - 7, -bestScore, false, depth - 1);
        if (score > bestScore) {
            bestScore = score;
            sq = tzcnt(sqbit);
        }
        moves ^= sqbit;
    }

    return sq;
}

int chooseRandomMove(Bitboard p, Bitboard o, std::mt19937 &mt) {
    Bitboard moves = getMoves(p, o);
    int mobility = popcount(moves);
    int chosen = mt() % mobility;

    for (int i = 0; i < chosen; ++i) {
        moves &= moves - 1ULL;
    }

    return tzcnt(moves);
}