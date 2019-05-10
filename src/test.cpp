#include <random>
#include "bitboard.h"
#include "engine.h"
#include "reversi.h"

void testBitboardOperation() {
    // テスト回数
    constexpr int N = 10000000;

    std::random_device rnd;
    std::mt19937_64 mt64(rnd());

    int diff_getmoves = 0;
    int diff_getflip = 0;

    for (int i = 0; i < N; ++i) {
        // ランダムな盤面を作成.（この盤面が再現可能かどうかは無視）
        Bitboard b = mt64();
        Bitboard w = mt64() & ~b;
        Bitboard blank = ~(b | w);

        Bitboard moves0 = getMoves(b, w);
        Bitboard moves1 = getMoves_slow(b, w);

        if (moves0 != moves1) ++diff_getmoves;

        int chosen = mt64() % popcount(blank);
        while (chosen--) blank &= blank - 1ULL;

        Bitboard sqbit = blank & -blank;

        Bitboard flip0 = getFlip(b, w, sqbit);
        Bitboard flip1 = getFlip_slow(b, w, sqbit);

        if (flip0 != flip1) {
            ++diff_getflip;

            printBitboard(flip0);
            printBitboard(flip1);
        }
    }

    std::cout << diff_getmoves << std::endl << diff_getflip << std::endl;
}

// ランダムAI VS engineの自動対戦
void play(int N, int depth) {
    std::random_device rnd;
    std::mt19937 mt(rnd());

    int wins = 0;

    for (int i = 0; i < N; ++i) {
        Color randomPlayer = (Color)(mt() & 1);
        Reversi reversi;
        while (!reversi.isFinished) {
            int sq;
            if (reversi.c == randomPlayer) {
                sq = chooseRandomMove(reversi.p, reversi. o, mt);
            }
            else {
                sq = chooseBestMove(reversi.p, reversi. o, depth);
            }
            reversi.move(sq);
        }

        if (reversi.winner() == ~randomPlayer) ++wins;
    }

    std::cout << "Win rate:" << ((double)wins / (double)N * 100) << "%" << std::endl;
}