#include "engine.h"
#include "eval.h"
#include "search.h"
#include <cassert>

RandomEngine::RandomEngine() : mt(std::random_device()()) {}

int RandomEngine::chooseMove(Bitboard p, Bitboard o, int depth) {
    Bitboard moves = getMoves(p, o);
    int mobility = popcount(moves);
    int chosen = mt() % mobility;

    for (int i = 0; i < chosen; ++i) {
        moves &= moves - 1ULL;
    }

    return tzcnt(moves);
}

AlphaBetaEngine::AlphaBetaEngine(const std::string &weightFolderpath) : evaluator(weightFolderpath) {}

double AlphaBetaEngine::negaAlpha(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : -negaAlpha(o, p, -beta, -alpha, true, depth);

    if (depth == 0)
        return evaluator.evaluate(p, o);

    double bestScore = -EvalInf;

    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        double score = -negaAlpha(o ^ flip, p ^ flip ^ sqbit, -beta, -bestScore, false, depth - 1);
        if (score >= beta)
            return score;

        bestScore = std::max(score, bestScore);
        moves ^= sqbit;
    }

    return bestScore;
}

std::vector<MoveWithScore> AlphaBetaEngine::evalAllMoves(Bitboard p, Bitboard o, int depth) {
    std::vector<MoveWithScore> movesWithScore;
    Bitboard moves = getMoves(p, o);

    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        double score = -negaAlpha(o ^ flip, p ^ flip ^ sqbit, -EvalInf, EvalInf, false, depth - 1);
        int sq = tzcnt(sqbit);
        movesWithScore.push_back({sq, score});
    }

    return movesWithScore;
}

int AlphaBetaEngine::chooseMove(Bitboard p, Bitboard o, int depth) {
    double bestScore = -EvalInf;
    int sq = -1;

    Bitboard moves = getMoves(p, o);
    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        double score = -negaAlpha(o ^ flip, p ^ flip ^ sqbit, -EvalInf, -bestScore, false, depth - 1);
        if (score >= bestScore) {
            bestScore = score;
            sq = tzcnt(sqbit);
        }
    }

    assert(sq != -1);
    return sq;
}

NegaScoutEngine::NegaScoutEngine(const std::string &weightFolderpath) : AlphaBetaEngine(weightFolderpath) {}

double NegaScoutEngine::negaScout(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth) {
    if (depth <= 3)
        return negaAlpha(p, o, alpha, beta, passed, depth);

    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : -negaScout(o, p, -beta, -alpha, true, depth);

    const PositionKey key = {p, o};
    bool isSearched = transpositionTable.count(key);
    SearchedPosition<double> &sp = transpositionTable[key];
    if (isSearched) {
        if (sp.lower >= beta)
            return sp.lower;
        if (sp.upper <= alpha)
            return sp.upper;
        if (sp.upper == sp.lower)
            return sp.upper;

        alpha = std::max(alpha, sp.lower);
        beta = std::min(beta, sp.upper);
    } else {
        sp = {EvalInf, -EvalInf, p, o};
    }

    std::vector<CandidateMove<double>> orderedMoves;
    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        Bitboard nextP = o ^ flip;
        Bitboard nextO = p ^ flip ^ sqbit;

        orderedMoves.push_back({nextP, nextO, evaluator.evaluate(nextP, nextO)});
    }

    // 敵から見て評価値の小さい順にみていく
    std::sort(orderedMoves.begin(), orderedMoves.end());

    double bestScore = -negaScout(orderedMoves[0].nextP, orderedMoves[0].nextO, -beta, -alpha, false, depth - 1);
    if (bestScore >= beta) {
        return sp.lower = std::max(sp.lower, bestScore);
    }

    double a = std::max(alpha, bestScore);
    for (int i = 1; i < orderedMoves.size(); ++i) {
        auto &cm = orderedMoves[i];
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
            return sp.lower = std::max(roughScore, sp.lower);
        } else if (roughScore <= a) {
            bestScore = std::max(bestScore, roughScore);
        } else if (roughScore >= (a + NWSWindowSize)) {
            a = bestScore = -negaScout(cm.nextP, cm.nextO, -beta, -roughScore, false, depth - 1);

            if (bestScore >= beta) {
                return sp.lower = std::max(sp.lower, bestScore);
            }
        }
        // (a, a+WindowSize)にroughScoreがあった場合。
        // roughScoreは局面(nextP, nextO)の真の評価値ということ
        else {
            a = bestScore = roughScore;
        }
    }

    if (bestScore <= alpha) {
        return sp.upper = std::min(sp.upper, bestScore);
    } else {
        return sp.lower = sp.upper = bestScore;
    }
}

std::vector<MoveWithScore> NegaScoutEngine::evalAllMoves(Bitboard p, Bitboard o, int depth) {
    transpositionTable.clear();
    std::vector<MoveWithScore> movesWithScore;
    Bitboard moves = getMoves(p, o);

    while (moves != 0ULL) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        double score = -negaScout(o ^ flip, p ^ flip ^ sqbit, -EvalInf, EvalInf, false, depth - 1);
        // double score_ = -negaAlpha(o ^ flip, p ^ flip ^ sqbit, -EvalInf, EvalInf, false, depth - 1);
        // assert(score == score_);
        int sq = tzcnt(sqbit);
        movesWithScore.push_back({sq, score});
    }

    return movesWithScore;
}

int NegaScoutEngine::chooseMove(Bitboard p, Bitboard o, int depth) {
    transpositionTable.clear();
    double bestScore = -EvalInf;
    int sq = -1;

    Bitboard moves = getMoves(p, o);
    while (moves != 0ULL) {
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