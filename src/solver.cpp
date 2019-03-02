#include "bitboard.h"
#include "solver.h"

// negamaxかいて. <- 要はpからみた石差の最大値を返す
static int searchFinal(Bitboard p, Bitboard o, int min, int max, bool passed) {
    // 全部埋まってるとき
    if (p | o == 0xffffffffffffffffULL) return popcount(p) - popcount(o);

    Bitboard moves = getMoves(p, o);

    // パスの処理
    if (moves == 0ULL) return passed ? (popcount(p) - popcount(o)) : -searchFinal(o, p, -max, -min, true);

    // 打てるところがあるとき
    
    int bestScore = -64; // 64が最小値なので
    // 下からビットを取り出していく
    for (Bitboard sqbit = moves & -moves; sqbit != 0ULL; moves ^= sqbit, sqbit = moves & -moves) {
        // 盤面をすすめる
        Bitboard flip = getFlip(p, o, sqbit);
        // 敵からみた石差が返ってくるため, 符号を反転させる
        int score = -searchFinal(o ^ flip, p ^ flip ^ sqbit, -max, -std::max(bestScore, min), false); // 下限がbestScore or minになる（大きい方）. 相手から見たときの符号反転注意

        // 枝刈りできます(少なくともbestScoreがmaxより大きくなるので)
        if (score > max) return bestScore;

        bestScore = std::max(score, bestScore);
    }

    return bestScore;
}

static int search(Bitboard p, Bitboard o, int min, int max, bool passed) {

}

int solve(Bitboard p, Bitboard o) {

}