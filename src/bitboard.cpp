#include <iostream>
#include <random>
#include "bitboard.h"

std::istream& operator >> (std::istream& is, Color& c) {
    int t; is >> t;
    c = (Color)t;
    return is;
}

// a <= x <= bか?
static bool within(int a, int b, int x) {
    return a <= x && x <= b;
}

Bitboard getMoves_slow(Bitboard p, Bitboard o) {
    Bitboard moves = 0ULL;

    for (int i = 0; i < 64; ++i) {
        // 空白マス以外はcontinue
        if ((p | o) >> i & 1ULL) continue;

        // 8方向みる
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                if (x == 0 && y == 0) continue;

                // 連続が切れるor端に到達したときにそのマスに自分の石があればひっくり返せる.
                int cur = i;
                int streak = 0;
                // curから調べている方向に1つ伸ばしたマスが盤面の外なら、curは端に位置している.
                while (within(0, 7, cur % 8 + x) && within(0, 7, cur / 8 + y)) {
                    if (cur != i) {
                        if (o >> cur & 1ULL) ++streak;
                        else break;
                    }

                    cur += x + y * 8;
                }

                // この方向にひっくり返せる
                // goto書くの嫌なので途中でひっくり返せると分かっても8方向全部見る
                if (streak > 0 && (p >> cur & 1ULL)) moves |= 1ULL << i;
            }
        }
    }

    return moves;
}

Bitboard getFlip_slow(Bitboard p, Bitboard o, Bitboard sqbit) {
    int sq = tzcnt(sqbit);
    Bitboard flip = 0ULL;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            if (x == 0 && y == 0) continue;

            int cur = sq;
            Bitboard fl = 0ULL;
            while (within(0, 7, cur % 8 + x) && within(0, 7, cur / 8 + y)) {
                if (cur != sq) {
                    if (o >> cur & 1ULL) fl |= 1ULL << cur;
                    else break;
                }

                cur += x + y * 8;
            }

            if (fl != 0ULL && (p >> cur & 1ULL)) flip |= fl;
        }
    }

    return flip;
}

void printBitboard(const Bitboard x) {
    for (int i = 63; i >= 0; --i) {
        std::cout << ((x >> i & 1) ? 'o' : '-');
        if (i % 8 != 0) std::cout << ' ';
        else std::cout << std::endl;
    }
}