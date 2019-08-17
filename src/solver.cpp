#include "solver.h"
#include "bitboard.h"
#include "reversi.h"
#include "search.h"
#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

static long long nodeCount = 0L;

// v = ミニマックス値

// negamaxかいて. <- 要はpからみた石差の最大値を返す
// alpha < v < beta
static int negamax(Bitboard p, Bitboard o, int alpha, int beta, bool passed) {
    ++nodeCount;

    if (popcount(p | o) == 63) {
        Bitboard flip = getFlip(p, o, ~(p | o));

        if (flip != 0ULL) {
            ++nodeCount;
            return popcount(p) - popcount(o) + (popcount(flip) << 1) + 1;
        } else {
            flip = getFlip(o, p, ~(p | o));
            if (flip != 0ULL) {
                ++nodeCount;
                return popcount(p) - popcount(o) - (popcount(flip) << 1) - 1;
            } else {
                return popcount(p) - popcount(o);
            }
        }
    }

    Bitboard moves = getMoves(p, o);

    // パスの処理
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : (--nodeCount, -negamax(o, p, -beta, -alpha, true));

    // 打てるところがあるとき

    int bestScore = -64; // 64が最小値なので
    // 下からビットを取り出していく
    while (moves) {
        Bitboard sqbit = moves & -moves;
        // 盤面をすすめる
        Bitboard flip = getFlip(p, o, sqbit);
        // 敵からみた石差が返ってくるため, 符号を反転させる
        int score = -negamax(o ^ flip, p ^ flip ^ sqbit, -beta, -std::max(bestScore, alpha), false); // 下限がbestScore or minになる（大きい方）. 相手から見たときの符号反転注意
        // 枝刈りできます(少なくともbestScoreがmax以上になるので)
        if (score >= beta)
            return score;
        bestScore = std::max(score, bestScore);
        moves ^= sqbit;
    }

    return bestScore;
}

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

SearchedPosition transpositionTable[TTSize];

static int negascout(Bitboard p, Bitboard o, int alpha, int beta, bool passed) {
    if (popcount(p | o) >= 58)
        return negamax(p, o, alpha, beta, passed);

    ++nodeCount;

    Bitboard moves = getMoves(p, o);
    // パス
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : (--nodeCount, -negascout(o, p, -beta, -alpha, true));

    // テーブルから前の探索結果を取り出し、vの範囲を狭めて効率化を図る.
    const int index = getIndex(p, o);
    SearchedPosition sp = transpositionTable[index];

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

    std::vector<CandidateMove> orderedMoves;
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
    int bestScore = -negascout(orderedMoves[0].nextP, orderedMoves[0].nextO, -beta, -alpha, false);

    // 枝刈り
    if (bestScore >= beta) {
        sp.lower = std::max(bestScore, sp.lower);
        // upperはそのまま

        // 更新
        transpositionTable[index] = sp;
        return sp.lower;
    }

    int a = std::max(bestScore, alpha); // 探索窓: (beta(bestScore, alpha), beta) minはつかうので改変しない
    for (int i = 1; i < orderedMoves.size(); ++i) {
        CandidateMove &cm = orderedMoves[i];
        // (1) v <= roughScore <= a
        // (2) a < roughScore <= v
        // のどちらかを満たす
        // Null Window Searchのこの性質が成り立つ理由がわからんけど... <- わかりましたいずれ証明をQiitaに上げます
        int roughScore = -negascout(cm.nextP, cm.nextO, -(a + 1), -a, false);

        // roughScore >= beta > a なので (2).
        if (roughScore >= beta) {
            sp.lower = std::max(roughScore, sp.lower);
            transpositionTable[index] = sp;
            return sp.lower;
        }
        // (2)、 再探索
        else if (roughScore > a) {
            // v => roughScore > a >= bestScoreであるので
            // ついでに探索窓も狭める
            a = bestScore = -negascout(cm.nextP, cm.nextO, -beta, -roughScore, false);

            // 枝刈りチェック
            if (bestScore >= beta) {
                sp.lower = std::max(bestScore, sp.lower);
                transpositionTable[index] = sp;
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
        transpositionTable[index] = sp;
        return bestScore;
    }
    // bestScore <= alpha
    else {
        sp.upper = std::min(bestScore, sp.upper);
        transpositionTable[index] = sp;
        return sp.upper;
    }
}

// 方針としてはまず最善結果を求めておいて、これをもとに途中計算を軽くする.
void findBestMoves(Bitboard p, Bitboard o, bool passed, int bestScore, std::vector<int> &ans) {
    Bitboard moves = getMoves(p, o);
    if (moves == 0ULL) {
        if (!passed)
            findBestMoves(o, p, true, -bestScore, ans);
        return;
    }

    while (moves) {
        Bitboard sqbit = moves & -moves;
        moves ^= sqbit;
        Bitboard flip = getFlip(p, o, sqbit);
        Bitboard _p = p ^ flip ^ sqbit;
        Bitboard _o = o ^ flip;
        /* alphabetaではmin < v < maxならばvは正確な評価値であることが確定するので、以下のように探索窓を設定しても
           この手がbestScoreを導く手かどうか分かる */
        int score = -negascout(_o, _p, -bestScore - 1, -bestScore + 1, false);
        if (score == bestScore) {
            ans.push_back(tzcnt(sqbit));
            p = _p;
            o = _o;
            break;
        }
    }
    // なにか値を返すわけではない
    return findBestMoves(o, p, false, -bestScore, ans);
}

Solution solve(Bitboard p, Bitboard o) {
    Solution solution;
    nodeCount = 0;

    auto start = std::chrono::system_clock::now();

    solution.bestScore = negascout(p, o, -64, 64, false);

    auto end1 = std::chrono::system_clock::now();
    solution.nodeCount = nodeCount;

    findBestMoves(p, o, false, solution.bestScore, solution.bestMoves);

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

    for (FFO ffo : tests) {
        Solution solution = solve(ffo.p, ffo.o);

        std::cout << "BestScore:" << solution.bestScore << std::endl
                  << "ScoreLockTime:" << (solution.scoreLockTime / 1000.0) << " sec" << std::endl
                  << "WholeTime:" << (solution.wholeTime / 1000.0) << " sec" << std::endl
                  << "NPS:" << solution.NPS() << std::endl;

        for (int sq : solution.bestMoves) {
            std::cout << convertToLegibleSQ(sq) << ' ';
        }

        std::cout << std::endl;
    }
}