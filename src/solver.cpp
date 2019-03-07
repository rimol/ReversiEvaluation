#include <algorithm>
#include <string>
#include <vector>
#include "bitboard.h"
#include "board.h"
#include "solver.h"

// negamaxかいて. <- 要はpからみた石差の最大値を返す
// min < v < max
static int negamax(Bitboard p, Bitboard o, int min, int max, bool passed) {
    // 全部埋まってるとき
    // if ((p | o) == 0xffffffffffffffffULL) return popcount(p) - popcount(o);

    Bitboard moves = getMoves(p, o);

    // パスの処理
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : -negamax(o, p, -max, -min, true);

    // 打てるところがあるとき
    
    int bestScore = -64; // 64が最小値なので
    // 下からビットを取り出していく
    while (moves) {
        Bitboard sqbit = moves & -moves;
        // 盤面をすすめる
        Bitboard flip = getFlip(p, o, sqbit);
        // 敵からみた石差が返ってくるため, 符号を反転させる
        int score = -negamax(o ^ flip, p ^ flip ^ sqbit, -max, -std::max(bestScore, min), false); // 下限がbestScore or minになる（大きい方）. 相手から見たときの符号反転注意
        // 枝刈りできます(少なくともbestScoreがmax以上になるので)
        if (score >= max) return bestScore;
        bestScore = std::max(score, bestScore);
        moves ^= sqbit;
    }

    return bestScore;
}

struct CandidateMove {
    Bitboard p;
    Bitboard o;
    int weightedMobility;

    // ソート用
    bool operator < (const CandidateMove& cm) {
        return weightedMobility < cm.weightedMobility;
    }
};

// TODO: 置換表
// weighted mobilityの少ない順から見ていく. null window search
static int negascout(Bitboard p, Bitboard o, int min, int max, bool passed) {
    if (popcount(p | o) >= 58) return negamax(p, o, min, max, passed);

    Bitboard moves = getMoves(p, o);
    // パス
    if (moves == 0ULL)
        return passed ? (popcount(p) - popcount(o)) : -negascout(o, p, -max, -min, true);

    // vector, どうなんやろなあ（重そう）
    // 最大着手数: https://www9.atwiki.jp/othello/pages/50.html 証明すごい..
    // とりあえず46あれば十分.
    std::vector<CandidateMove> orderedMoves;

    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);

        Bitboard nextp = p ^ flip ^ sqbit;
        Bitboard nexto = o ^ flip;

        Bitboard opponentsMoves = getMoves(nexto, nextp);
        // 角に+1重みをつけた敵の置けるところの数
        int weightedMobility = popcount(opponentsMoves)
            + (opponentsMoves >> 0 & 1ULL)
            + (opponentsMoves >> 7 & 1ULL)
            + (opponentsMoves >> 56 & 1ULL)
            + (opponentsMoves >> 63 & 1ULL);

        orderedMoves.push_back({nextp, nexto, weightedMobility});
        moves ^= sqbit;
    }

    // 少ない順=昇順
    // ここでは使ってないが http://stlalv.la.coocan.jp/sort.html
    std::sort(orderedMoves.begin(), orderedMoves.end());

    // ここから探索
    CandidateMove probablyBestMove = orderedMoves[0];
    int bestScore = -negascout(probablyBestMove.o, probablyBestMove.p, -max, -min, false);

    // 枝刈り
    if (bestScore >= max) {
        return bestScore;
    }

    const int len = orderedMoves.size();
    int a = std::max(bestScore, min); // 探索窓: (max(bestScore, min), max) minはつかうので改変しない
    for (int i = 1; i < len; ++i) {
        CandidateMove cm = orderedMoves[i];
        // (1) v <= roughScore <= a
        // (2) a < roughScore <= v
        // のどちらかを満たす
        // Null Window Searchのこの性質が成り立つ理由がわからんけど...
        int roughScore = -negascout(cm.o, cm.p, -(a + 1), -a, false);

        // roughScore >= max > a なので (2).
        if (roughScore >= max) {
            return roughScore;
        }
        // (2)、 再探索
        else if (roughScore > a) {
            // v => roughScore > a >= bestScoreであるので
            // ついでに探索窓も狭める
            a = bestScore = -negascout(cm.o, cm.p, -max, -roughScore, false);

            // 枝刈りチェック
            if (bestScore >= max) {
                return bestScore;
            }
        }
        // roughScore <= a, つまり(1)
        // この場合もbestScoreは更新される可能性がある. (bestScore <= minのとき)
        else {
            bestScore = std::max(roughScore, bestScore);
        }
    }

    return bestScore;
}

int solve(Bitboard p, Bitboard o) {
    return negascout(p, o, -64, 64, false);
}

struct FFO {
    Color c;
    Bitboard bits[2];

    // X: 黒, O: 白, -: 空白マス
    FFO(std::string boardText, Color _c) : c(_c), bits({0ULL, 0ULL}) {
        Bitboard bit = 1ULL;
        for (char c : boardText) {
            if (c == 'X') bits[Black] |= bit;
            else if (c == 'O') bits[White] |= bit;

            bit <<= 1;
        }
    }
};

void ffotest() {
    const FFO boards[] = {
        { "O--OOOOX-OOOOOOXOOXXOOOXOOXOOOXXOOOOOOXX---OOOOX----O--X--------", Black },
        { "-OOOOO----OOOOX--OOOOOO-XXXXXOO--XXOOX--OOXOXX----OXXO---OOO--O-", Black },
    };

    for (FFO ffo : boards) {
        Board b(ffo.c, ffo.bits[Black], ffo.bits[White]);
        printBoard(b);
        std::cout << solve(ffo.bits[ffo.c], ffo.bits[~ffo.c]) << std::endl;
    }
}