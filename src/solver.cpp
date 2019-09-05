#include "solver.h"
#include "bitboard.h"
#include "reversi.h"
#include "search.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <vector>

Solver::Solver(const Evaluator &evaluator) : nodeCount(), evaluator(evaluator) {
    transpositionTable2 = new SearchedPosition<int>[TTSize];
}

Solver::~Solver() {
    delete[] transpositionTable2;
}

void Solver::clear() {
    nodeCount = 0;
    transpositionTable1.clear();
    std::memset(transpositionTable2, 0, sizeof(SearchedPosition<int>) * TTSize);
}

int Solver::negaAlpha1(Bitboard p, Bitboard o, int alpha, int beta, int depth, bool passed) {
    if (depth <= 3)
        return negaAlpha2(p, o, alpha, beta, depth, passed);

    ++nodeCount;

    Bitboard moves = getMoves(p, o);

    // パスの処理
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : (--nodeCount, -negaAlpha1(o, p, -beta, -alpha, depth, true));

    // 奇数空きの区画から打つ方針。（偶数理論）
    Bitboard betterMoves = 0ULL;
    Bitboard t;
    t = moves & LeftTop;
    betterMoves |= t & static_cast<Bitboard>(-parity(t));
    t = moves & LeftBottom;
    betterMoves |= t & static_cast<Bitboard>(-parity(t));
    t = moves & RightTop;
    betterMoves |= t & static_cast<Bitboard>(-parity(t));
    t = moves & RightBottom;
    betterMoves |= t & static_cast<Bitboard>(-parity(t));

    moves ^= betterMoves;

    int bestScore = -64;

    while (betterMoves) {
        Bitboard sqbit = betterMoves & -betterMoves;
        betterMoves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        int score = -negaAlpha1(o ^ flip, p ^ flip ^ sqbit, -beta, -std::max(bestScore, alpha), depth - 1, false);
        if (score >= beta)
            return score;
        bestScore = std::max(score, bestScore);
    }

    while (moves) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        int score = -negaAlpha1(o ^ flip, p ^ flip ^ sqbit, -beta, -std::max(bestScore, alpha), depth - 1, false);
        if (score >= beta)
            return score;
        bestScore = std::max(score, bestScore);
    }

    return bestScore;
}

int Solver::negaAlpha2(Bitboard p, Bitboard o, int alpha, int beta, int depth, bool passed) {
    assert(depth > 0);
    ++nodeCount;

    if (depth == 1) {
        int emptySQ = tzcnt(~(p | o));
        int numFlip = countFlip(p, emptySQ);

        if (numFlip != 0ULL)
            return popcount(p) - popcount(o) + numFlip * 2 + 1;

        numFlip = countFlip(o, emptySQ);
        return numFlip != 0ULL ? (popcount(p) - popcount(o) - numFlip * 2 - 1) : (popcount(p) - popcount(o));
    }

    Bitboard moves = getMoves(p, o);

    // パスの処理
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : (--nodeCount, -negaAlpha2(o, p, -beta, -alpha, depth, true));

    // 打てるところがあるとき

    int bestScore = -64; // 64が最小値なので
    // 下からビットを取り出していく
    while (moves) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        // 盤面をすすめる
        Bitboard flip = getFlip(p, o, sqbit);
        // 敵からみた石差が返ってくるため, 符号を反転させる
        int score = -negaAlpha2(o ^ flip, p ^ flip ^ sqbit, -beta, -std::max(bestScore, alpha), depth - 1, false); // 下限がbestScore or minになる（大きい方）. 相手から見たときの符号反転注意
        // 枝刈りできます(少なくともbestScoreがmax以上になるので)
        if (score >= beta)
            return score;
        bestScore = std::max(score, bestScore);
    }

    return bestScore;
}

int Solver::negaScout2(Bitboard p, Bitboard o, int alpha, int beta, int depth, bool passed) {
    if (depth <= 7)
        return negaAlpha1(p, o, alpha, beta, depth, passed);

    ++nodeCount;

    Bitboard moves = getMoves(p, o);
    // パス
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : (--nodeCount, -negaScout2(o, p, -beta, -alpha, depth, true));

    // テーブルから前の探索結果を取り出し、vの範囲を狭めて効率化を図る.
    const int index = getIndex(p, o);
    auto sp = transpositionTable2[index];

    if (sp.correspondsTo(p, o)) {
        // カット
        if (sp.lower >= beta)
            return sp.lower;
        else if (sp.upper <= alpha)
            return sp.upper;
        else if (sp.upper == sp.lower)
            return sp.upper;

        // 窓縮小
        alpha = std::max(alpha, sp.lower);
        beta = std::min(beta, sp.upper);
    }
    // 一致しなかったorはじめて
    else
        sp = {64, -64, p, o};

    std::vector<CandidateMove<int>> orderedMoves;
    while (moves) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        Bitboard nextP = o ^ flip;
        Bitboard nextO = p ^ flip ^ sqbit;
        orderedMoves.push_back({nextP, nextO, getWeightedMobility(nextP, nextO)});
    }

    // 少ない順=昇順
    // ここでは使ってないが http://stlalv.la.coocan.jp/sort.html
    std::sort(orderedMoves.begin(), orderedMoves.end());

    // ここから探索
    int bestScore = -negaScout2(orderedMoves[0].nextP, orderedMoves[0].nextO, -beta, -alpha, depth - 1, false);

    // 枝刈り
    if (bestScore >= beta) {
        sp.lower = std::max(bestScore, sp.lower);
        // upperはそのまま

        // 更新
        transpositionTable2[index] = sp;
        return sp.lower;
    }

    int a = std::max(bestScore, alpha); // 探索窓: (beta(bestScore, alpha), beta) minはつかうので改変しない
    for (int i = 1; i < orderedMoves.size(); ++i) {
        auto &cm = orderedMoves[i];
        // (1) v <= roughScore <= a
        // (2) a < roughScore <= v
        // のどちらかを満たす
        // Null Window Searchのこの性質が成り立つ理由がわからんけど... <- わかりましたいずれ証明をQiitaに上げます
        int roughScore = -negaScout2(cm.nextP, cm.nextO, -(a + 1), -a, depth - 1, false);

        // roughScore >= beta > a なので (2).
        if (roughScore >= beta) {
            sp.lower = std::max(roughScore, sp.lower);
            transpositionTable2[index] = sp;
            return sp.lower;
        }
        // (2)、 再探索
        else if (roughScore > a) {
            // v => roughScore > a >= bestScoreであるので
            // ついでに探索窓も狭める
            a = bestScore = -negaScout2(cm.nextP, cm.nextO, -beta, -roughScore, depth - 1, false);

            // 枝刈りチェック
            if (bestScore >= beta) {
                sp.lower = std::max(bestScore, sp.lower);
                transpositionTable2[index] = sp;
                return sp.lower;
            }
        }
        // roughScore <= a, つまり(1)
        // この場合もbestScoreは更新される可能性がある. (bestScore <= minのとき)
        else {
            bestScore = std::max(roughScore, bestScore);
        }
    }

    // 置換表更新
    // 確定
    if (bestScore > alpha) {
        sp.upper = sp.lower = bestScore;
        transpositionTable2[index] = sp;
        return bestScore;
    }
    // bestScore <= alpha
    else {
        sp.upper = std::min(bestScore, sp.upper);
        transpositionTable2[index] = sp;
        return sp.upper;
    }
}

int Solver::negaScout1(Bitboard p, Bitboard o, int alpha, int beta, int depth, bool passed) {
    if (depth <= 15)
        return negaScout2(p, o, alpha, beta, depth, passed);

    ++nodeCount;

    Bitboard moves = getMoves(p, o);
    // パス
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : (--nodeCount, -negaScout1(o, p, -beta, -alpha, depth, true));

    // テーブルから前の探索結果を取り出し、vの範囲を狭めて効率化を図る.
    const PositionKey key = {p, o};
    bool isSearched = transpositionTable1.count(key);
    SearchedPosition<int> &sp = transpositionTable1[key];

    if (isSearched) {
        // カット
        if (sp.lower >= beta)
            return sp.lower;
        else if (sp.upper <= alpha)
            return sp.upper;
        else if (sp.upper == sp.lower)
            return sp.upper;

        // 窓縮小
        alpha = std::max(alpha, sp.lower);
        beta = std::min(beta, sp.upper);
    }
    // 一致しなかったorはじめて
    else
        sp = {64, -64, p, o};

    std::vector<CandidateMove<double>> orderedMoves;
    while (moves) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        Bitboard nextP = o ^ flip;
        Bitboard nextO = p ^ flip ^ sqbit;
        orderedMoves.push_back({nextP, nextO, evaluator.evaluate(nextP, nextO)});
    }

    // 少ない順=昇順
    // ここでは使ってないが http://stlalv.la.coocan.jp/sort.html
    std::sort(orderedMoves.begin(), orderedMoves.end());

    // ここから探索
    int bestScore = -negaScout1(orderedMoves[0].nextP, orderedMoves[0].nextO, -beta, -alpha, depth - 1, false);

    // 枝刈り
    if (bestScore >= beta) {
        sp.lower = std::max(bestScore, sp.lower);
        // upperはそのまま

        // 更新
        transpositionTable1[key] = sp;
        return sp.lower;
    }

    int a = std::max(bestScore, alpha); // 探索窓: (beta(bestScore, alpha), beta) minはつかうので改変しない
    for (int i = 1; i < orderedMoves.size(); ++i) {
        auto &cm = orderedMoves[i];
        // (1) v <= roughScore <= a
        // (2) a < roughScore <= v
        // のどちらかを満たす
        // Null Window Searchのこの性質が成り立つ理由がわからんけど... <- わかりましたいずれ証明をQiitaに上げます
        int roughScore = -negaScout1(cm.nextP, cm.nextO, -(a + 1), -a, depth - 1, false);

        // roughScore >= beta > a なので (2).
        if (roughScore >= beta) {
            sp.lower = std::max(roughScore, sp.lower);
            transpositionTable1[key] = sp;
            return sp.lower;
        }
        // (2)、 再探索
        else if (roughScore > a) {
            // v => roughScore > a >= bestScoreであるので
            // ついでに探索窓も狭める
            a = bestScore = -negaScout1(cm.nextP, cm.nextO, -beta, -roughScore, depth - 1, false);

            // 枝刈りチェック
            if (bestScore >= beta) {
                sp.lower = std::max(bestScore, sp.lower);
                transpositionTable1[key] = sp;
                return sp.lower;
            }
        }
        // roughScore <= a, つまり(1)
        // この場合もbestScoreは更新される可能性がある. (bestScore <= minのとき)
        else {
            bestScore = std::max(roughScore, bestScore);
        }
    }

    // 置換表更新
    // 確定
    if (bestScore > alpha) {
        sp.upper = sp.lower = bestScore;
        transpositionTable1[key] = sp;
        return bestScore;
    }
    // bestScore <= alpha
    else {
        sp.upper = std::min(bestScore, sp.upper);
        transpositionTable1[key] = sp;
        return sp.upper;
    }
}

// 方針としてはまず最善結果を求めておいて、これをもとに途中計算を軽くする.
std::vector<int> Solver::getBestMoves(Bitboard p, Bitboard o, int bestScore) {
    std::vector<int> bestMoves;
    Reversi reversi(p, o);

    while (!reversi.isFinished) {
        Bitboard moves = reversi.moves;
        int depth = 64 - reversi.stoneCount();

        if (depth == 1) {
            bestMoves.push_back(tzcnt(moves));
            break;
        }

        while (moves != 0ULL) {
            Bitboard sqbit = moves & -moves;
            moves ^= sqbit;
            Bitboard flip = getFlip(reversi.p, reversi.o, sqbit);
            Bitboard _p = reversi.p ^ flip ^ sqbit;
            Bitboard _o = reversi.o ^ flip;
            /* alphabetaではmin < v < maxならばvは正確な評価値であることが確定するので、以下のように探索窓を設定しても
           この手がbestScoreを導く手かどうか分かる */
            int score = -negaScout1(_o, _p, -bestScore - 1, -bestScore + 1, depth - 1, false);
            if (score == bestScore) {
                int sq = tzcnt(sqbit);
                bestMoves.push_back(sq);
                Color prevC = reversi.c;
                reversi.move(sq);
                Color curC = reversi.c;

                if (prevC != curC)
                    bestScore *= -1;
                break;
            }
        }
    }

    return bestMoves;
}

Solution Solver::solve(Bitboard p, Bitboard o) {
    clear();
    Solution solution;

    auto start = std::chrono::system_clock::now();
    solution.bestScore = negaScout1(p, o, -64, 64, 64 - popcount(p | o), false);
    auto end1 = std::chrono::system_clock::now();
    solution.nodeCount = nodeCount;
    solution.bestMoves = getBestMoves(p, o, solution.bestScore);
    auto end2 = std::chrono::system_clock::now();
    solution.scoreLockTime = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
    solution.wholeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();

    return solution;
}

struct FFO {
    Bitboard p, o;

    // X: 黒, O: 白, -: 空白マス
    FFO(std::string boardText, Color color) : p(), o() {
        Bitboard bit = 1ULL << 63;
        for (char c : boardText) {
            if (c == 'X')
                p |= bit;
            else if (c == 'O')
                o |= bit;

            bit >>= 1;
        }
        if (color == White)
            std::swap(p, o);
    }
};

void ffotest() {
    const FFO tests[] = {
        {"O--OOOOX-OOOOOOXOOXXOOOXOOXOOOXXOOOOOOXX---OOOOX----O--X--------", Black},
        {"-OOOOO----OOOOX--OOOOOO-XXXXXOO--XXOOX--OOXOXX----OXXO---OOO--O-", Black},
        {"--OOO-------XX-OOOOOOXOO-OOOOXOOX-OOOXXO---OOXOO---OOOXO--OOOO--", Black},
        {"--XXXXX---XXXX---OOOXX---OOXXXX--OOXXXO-OOOOXOO----XOX----XXXXX-", White},
        {"--O-X-O---O-XO-O-OOXXXOOOOOOXXXOOOOOXX--XXOOXO----XXXX-----XXX--", White},
        {"---XXXX-X-XXXO--XXOXOO--XXXOXO--XXOXXO---OXXXOO-O-OOOO------OO--", Black},
        {"---XXX----OOOX----OOOXX--OOOOXXX--OOOOXX--OXOXXX--XXOO---XXXX-O-", Black},
        {"-OOOOO----OOOO---OOOOX--XXXXXX---OXOOX--OOOXOX----OOXX----XXXX--", White},
    };

    std::string weightFolderpath;
    std::cout << "weight folder path:";
    std::cin >> weightFolderpath;

    PatternEvaluator evaluator(weightFolderpath);
    Solver solver(evaluator);

    for (FFO ffo : tests) {
        Solution solution = solver.solve(ffo.p, ffo.o);

        std::cout << "BestScore:" << solution.bestScore << std::endl
                  << "ScoreLockTime:" << (solution.scoreLockTime / 1000.0) << " sec" << std::endl
                  << "WholeTime:" << (solution.wholeTime / 1000.0) << " sec" << std::endl
                  << "Nodes:" << solution.nodeCount << std::endl
                  << "NPS:" << solution.NPS() << std::endl;

        for (int sq : solution.bestMoves) {
            std::cout << convertToLegibleSQ(sq) << ' ';
        }

        std::cout << std::endl;
    }
}