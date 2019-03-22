#include "engine.h"

int chooseBestMove(Bitboard p, Bitboard o) {
    // 下で呼ぶ評価関数で使う評価値を読み込む
    int stoneCount = popcount(p | o);
    // この番の自分の手を打ったあとの相手の手番における評価値を計算するので+1する
    changeEvaluationTables(stoneCount - 4 + 1);

    int minscore = 64;
    int sq = -1;

    Bitboard moves = getMoves(p, o);
    while (moves) {
        Bitboard sqbit = moves & -moves;
        Bitboard flip = getFlip(p, o, sqbit);
        // 石をおいた次の手番のoからみた評価値を計算する。
        // これが最小になるような手が最も良い手。
        int score = evaluate(o ^ flip, p ^ flip ^ sqbit);
        if (score < minscore) {
            minscore = score;
            sq = tzcnt(sqbit);
        }
        moves ^= sqbit;
    }

    return sq;
}