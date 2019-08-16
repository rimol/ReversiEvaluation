#include "engine.h"
#include "eval.h"
#include "search.h"
#include <cassert>

// todo 置換表
static double negaScout(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    if (depth == 0)
        return evaluate(p, o);

    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL)
        return passed ? evaluate(p, o) : -negaScout(o, p, -beta, -alpha, true, depth);

    std::vector<CandidateMove> orderedMoves;
    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        Bitboard nextP = o ^ flip;
        Bitboard nextO = p ^ flip ^ sqbit;
        orderedMoves.push_back({nextP, nextO, (int)evaluate(nextP, nextO)});
    }

    // 敵から見て評価値の小さい順にみていく
    std::sort(orderedMoves.begin(), orderedMoves.end());

    double bestScore = -negaScout(orderedMoves[0].nextP, orderedMoves[0].nextO, -beta, -alpha, false, depth - 1);
    if (bestScore >= beta) {
        return bestScore;
    }

    double a = std::max(alpha, bestScore);
    for (int i = 1; i < orderedMoves.size(); ++i) {
        CandidateMove &cm = orderedMoves[i];
        /*
        これ、評価値が実数なので1より少し幅を小さくしたほうがいいかも.
        ということで0.001ぐらいにしてみる.

        以下NWSでなぜalpha=betaとしないかの自分なりの考察。
        NWSでalpha以下かbeta以上であることがわかる（ここだとa以下かa+0.001以上）ので、
        もしalpha=betaとするとroughScoreでalphaが帰ってきたときに真の評価値が結局alpha以上かalpha以下かがわからない。
        よって微妙にずらす（あいだに真の評価値がこないぐらいの幅をもたせる）必要がある。
        */
        constexpr double NWSWindowSize = 0.001;
        double roughScore = -negaScout(cm.nextP, cm.nextO, -a - NWSWindowSize, -a, false, depth - 1);

        if (roughScore >= beta) {
            return roughScore;
        } else if (roughScore <= a) {
            bestScore = std::max(bestScore, roughScore);
        } else if (roughScore >= (a + NWSWindowSize)) {
            double score = -negaScout(cm.nextP, cm.nextO, -beta, -roughScore, false, depth - 1);

            if (score >= beta) {
                return score;
            }

            bestScore = std::max(score, bestScore);
        }
        // (a, a+WindowSize)にroughScoreがあった場合。
        // roughScoreは局面(nextP, nextO)の真の評価値ということ
        // あとから考えればこれはroughScore <= aと同じだなぁ
        else {
            bestScore = std::max(bestScore, roughScore);
        }
    }

    return bestScore;
}

double alphabeta(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    if (depth == 0)
        return evaluate(p, o);

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

    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);

        int sq = tzcnt(sqbit);
        double score = -negaScout(o ^ flip, p ^ flip ^ sqbit, -EvalInf, EvalInf, false, depth - 1);
        movesWithScore.push_back({sq, score});
    }

    return movesWithScore;
}

int chooseBestMove(Bitboard p, Bitboard o, int depth) {
    double bestScore = -EvalInf;
    int sq = -1;

    Bitboard moves = getMoves(p, o);
    while (moves) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        double score = -negaScout(o ^ flip, p ^ flip ^ sqbit, -EvalInf, -bestScore, false, depth - 1);
        if (score >= bestScore) {
            bestScore = score;
            sq = tzcnt(sqbit);
        }
    }

    assert(sq != -1);
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