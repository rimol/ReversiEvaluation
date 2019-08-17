#include "engine.h"
#include "eval.h"
#include "search.h"
#include <cassert>

// solverのやつは整数なのでわざわざもう一つdouble版をつくりました。
// テンプレート？そんなもん知らん。
struct SearchedPosition {
    double upper;
    double lower;

    Bitboard p;
    Bitboard o;

    bool correspondsTo(Bitboard _p, Bitboard _o) {
        return _p == p && _o == o;
    }
};

static SearchedPosition transpositonTable[TTSize];

static void clearTranspositionTable() {
    std::memset(transpositonTable, 0, sizeof(transpositonTable));
}

static double negaScout(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL && !passed)
        return -negaScout(o, p, -beta, -alpha, true, depth);

    const int index = getIndex(p, o);
    SearchedPosition sp = transpositonTable[index];

    if (sp.correspondsTo(p, o)) {
        if (sp.lower >= beta)
            return sp.lower;
        if (sp.upper <= alpha)
            return sp.upper;
        if (sp.upper == sp.lower)
            return sp.upper;

        alpha = std::max(alpha, sp.lower);
        beta = std::min(beta, sp.upper);
    } else
        sp = {EvalInf, -EvalInf, p, o};

    if (depth == 0 || moves == 0ULL) {
        sp.upper = sp.lower = evaluate(p, o);
        transpositonTable[index] = sp;
        return sp.upper;
    }

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
        sp.lower = std::max(sp.lower, bestScore);
        transpositonTable[index] = sp;
        return sp.lower;
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
            sp.lower = std::max(roughScore, sp.lower);
            transpositonTable[index] = sp;
            return sp.lower;
        } else if (roughScore <= a) {
            bestScore = std::max(bestScore, roughScore);
        } else if (roughScore >= (a + NWSWindowSize)) {
            a = bestScore = -negaScout(cm.nextP, cm.nextO, -beta, -roughScore, false, depth - 1);

            if (bestScore >= beta) {
                sp.lower = std::max(sp.lower, bestScore);
                transpositonTable[index] = sp;
                return sp.lower;
            }
        }
        // (a, a+WindowSize)にroughScoreがあった場合。
        // roughScoreは局面(nextP, nextO)の真の評価値ということ
        else {
            a = bestScore = roughScore;
        }
    }

    if (bestScore <= alpha) {
        sp.upper = std::min(sp.upper, bestScore);
        transpositonTable[index] = sp;
        return sp.upper;
    } else {
        sp.lower = sp.upper = bestScore;
        transpositonTable[index] = sp;
        return sp.lower;
    }
}

static double alphabeta(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL)
        return passed ? evaluate(p, o) : -alphabeta(o, p, -beta, -alpha, true, depth);

    if (depth == 0)
        return evaluate(p, o);

    double bestScore = -EvalInf;

    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        double score = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -beta, -bestScore, false, depth - 1);
        if (score >= beta)
            return score;

        bestScore = std::max(score, bestScore);
        moves ^= sqbit;
    }

    return bestScore;
}

std::vector<MoveWithScore> evalAllMoves(Bitboard p, Bitboard o, int depth) {
    clearTranspositionTable();

    std::vector<MoveWithScore> movesWithScore;
    Bitboard moves = getMoves(p, o);

    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);

        int sq = tzcnt(sqbit);
        double score = -negaScout(o ^ flip, p ^ flip ^ sqbit, -EvalInf, EvalInf, false, depth - 1);
        double score_ = -alphabeta(o ^ flip, p ^ flip ^ sqbit, -EvalInf, EvalInf, false, depth - 1);
        assert(score == score_);
        movesWithScore.push_back({sq, score});
    }

    return movesWithScore;
}

int chooseBestMove(Bitboard p, Bitboard o, int depth) {
    clearTranspositionTable();

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