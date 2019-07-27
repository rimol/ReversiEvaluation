#include "bitboard.h"
#include "engine.h"
#include "eval.h"
#include "reversi.h"
#include <cassert>
#include <random>
#include <string>

constexpr Bitboard makeBB(const int (&bits)[8]) {
    Bitboard bb = 0ULL;
    for (int i = 0; i < 8; ++i) {
        bb |= (Bitboard)(bits[7 - i] & 0b11111111) << (i * 8);
    }
    return bb;
}

int base3Tobase10(std::string s) {
    int x = 0;
    for (int i = 0; i < s.size(); ++i) {
        x += s[i] - '0';
        if (i < s.size() - 1)
            x *= 3;
    }
    return x;
}

void test() {
    // rotation
    assert(
        makeBB({
            0b00000001,
            0b00000010,
            0b00000100,
            0b00001000,
            0b00010000,
            0b00100000,
            0b01000000,
            0b10000000,
        }) ==
        rotateRightBy90(makeBB({
            0b10000000,
            0b01000000,
            0b00100000,
            0b00010000,
            0b00001000,
            0b00000100,
            0b00000010,
            0b00000001,
        })));

    assert(
        makeBB({
            0b00100000,
            0b01000000,
            0b10000000,
            0b00000000,
            0b00000000,
            0b11000000,
            0b11000000,
            0b00000000,
        }) ==
        rotateRightBy90(makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b10000000,
            0b01000110,
            0b00100110,
        })));

    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00001111,
            0b00001111,
            0b00001111,
            0b00001111,
        }) ==
        rotateRightBy90(makeBB({
            0b00001111,
            0b00001111,
            0b00001111,
            0b00001111,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
        })));

    assert(
        makeBB({
            0b11110000,
            0b11111010,
            0b11110000,
            0b11111010,
            0b01010011,
            0b00000011,
            0b01010000,
            0b00000000,
        }) ==
        rotateRightBy90(makeBB({
            0b00001100,
            0b01011100,
            0b00000000,
            0b01010000,
            0b11111010,
            0b11110000,
            0b11111010,
            0b11110000,
        })));

    assert(
        makeBB({
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
        }) ==
        rotateRightBy90(makeBB({
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
        })));

    assert(
        makeBB({
            0b10000000,
            0b00000001,
            0b10000000,
            0b00000001,
            0b10000000,
            0b00000001,
            0b10000000,
            0b00000001,
        }) ==
        rotateBy180(makeBB({
            0b10000000,
            0b00000001,
            0b10000000,
            0b00000001,
            0b10000000,
            0b00000001,
            0b10000000,
            0b00000001,
        })));

    assert(
        makeBB({
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
        }) ==
        rotateBy180(makeBB({
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
            0b11111111,
        })));

    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b10000000,
            0b01000000,
            0b00100000,
            0b00010000,
        }) ==
        rotateBy180(makeBB({
            0b00001000,
            0b00000100,
            0b00000010,
            0b00000001,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
        })));

    assert(
        makeBB({
            0b00001000,
            0b00001000,
            0b00000000,
            0b00001000,
            0b00000000,
            0b00001000,
            0b00001000,
            0b00001000,
        }) ==
        rotateBy180(makeBB({
            0b00010000,
            0b00010000,
            0b00010000,
            0b00000000,
            0b00010000,
            0b00000000,
            0b00010000,
            0b00010000,
        })));

    // pext
    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000010,
            0b01010110,
        }) ==
        pext(
            makeBB({
                0b00000000,
                0b00000000,
                0b00000000,
                0b00011000,
                0b11111111,
                0b00001111,
                0b11111100,
                0b01010110,
            }),
            makeBB({
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b01000010,
                0b11111111,
            })));

    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11111000,
        }) ==
        pext(
            makeBB({
                0b11111111,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000011,
                0b11111000,
                0b11111111,
            }),
            makeBB({
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b11111111,
                0b00000000,
            })));

    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b10111110,
        }) ==
        pext(
            makeBB({
                0b00001000,
                0b00001000,
                0b00001000,
                0b00111000,
                0b00000111,
                0b10111110,
                0b01000100,
                0b00010000,
            }),
            makeBB({
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b11111111,
                0b00000000,
                0b00000000,
            })));

    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11010101,
        }) ==
        pext(
            makeBB({
                0b10000000,
                0b00000000,
                0b00000000,
                0b11111111,
                0b11010101,
                0b11111111,
                0b00000000,
                0b00000001,
            }),
            makeBB({
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b11111111,
                0b00000000,
                0b00000000,
                0b00000000,
            })));

    assert(
        makeBB({
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11010111,
        }) ==
        pext(
            makeBB({
                0b00000001,
                0b00011110,
                0b00000000,
                0b00111000,
                0b00001110,
                0b00100000,
                0b01000000,
                0b11111111,
            }),
            makeBB({
                0b00000001,
                0b00000010,
                0b00000100,
                0b00001000,
                0b00010000,
                0b00100000,
                0b01000000,
                0b10000000,
            })));

    // SymmetricPattern
    assert(SymmetricPattern[0][base3Tobase10("2221011111")] == base3Tobase10("1111101222"));
    assert(SymmetricPattern[0][base3Tobase10("2211011101")] == base3Tobase10("1101101212"));
    assert(SymmetricPattern[0][base3Tobase10("0122112120")] == base3Tobase10("0122112120"));
    assert(SymmetricPattern[0][base3Tobase10("0211111101")] == base3Tobase10("0211111101"));
    assert(SymmetricPattern[0][base3Tobase10("0000000000")] == base3Tobase10("0000000000"));
    assert(SymmetricPattern[0][base3Tobase10("1111111111")] == base3Tobase10("1111111111"));
    assert(SymmetricPattern[0][base3Tobase10("2222222222")] == base3Tobase10("2222222222"));

    assert(SymmetricPattern[1][base3Tobase10("22222211")] == base3Tobase10("11222222"));
    assert(SymmetricPattern[1][base3Tobase10("11222222")] == base3Tobase10("11222222"));
    assert(SymmetricPattern[1][base3Tobase10("11222211")] == base3Tobase10("11222211"));
    assert(SymmetricPattern[1][base3Tobase10("02121210")] == base3Tobase10("01212120"));
    assert(SymmetricPattern[1][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    assert(SymmetricPattern[1][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    assert(SymmetricPattern[1][base3Tobase10("22222222")] == base3Tobase10("22222222"));

    assert(SymmetricPattern[4][base3Tobase10("212122201")] == base3Tobase10("122021212"));
    assert(SymmetricPattern[4][base3Tobase10("200120112")] == base3Tobase10("200120112"));
    assert(SymmetricPattern[4][base3Tobase10("101010102")] == base3Tobase10("101010102"));
    assert(SymmetricPattern[4][base3Tobase10("000000000")] == base3Tobase10("000000000"));
    assert(SymmetricPattern[4][base3Tobase10("111111111")] == base3Tobase10("111111111"));
    assert(SymmetricPattern[4][base3Tobase10("222222222")] == base3Tobase10("222222222"));

    assert(SymmetricPattern[5][base3Tobase10("22221111")] == base3Tobase10("11112222"));
    assert(SymmetricPattern[5][base3Tobase10("11112222")] == base3Tobase10("11112222"));
    assert(SymmetricPattern[5][base3Tobase10("10222201")] == base3Tobase10("10222201"));
    assert(SymmetricPattern[5][base3Tobase10("02121210")] == base3Tobase10("01212120"));
    assert(SymmetricPattern[5][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    assert(SymmetricPattern[5][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    assert(SymmetricPattern[5][base3Tobase10("22222222")] == base3Tobase10("22222222"));

    assert(SymmetricPattern[9][base3Tobase10("2120")] == base3Tobase10("0212"));
    assert(SymmetricPattern[9][base3Tobase10("1202")] == base3Tobase10("1202"));
    assert(SymmetricPattern[9][base3Tobase10("2002")] == base3Tobase10("2002"));
    assert(SymmetricPattern[9][base3Tobase10("0210")] == base3Tobase10("0120"));
    assert(SymmetricPattern[9][base3Tobase10("0000")] == base3Tobase10("0000"));
    assert(SymmetricPattern[9][base3Tobase10("1111")] == base3Tobase10("1111"));
    assert(SymmetricPattern[9][base3Tobase10("2222")] == base3Tobase10("2222"));

    assert(SymmetricPattern[10][base3Tobase10("2012010201")] == base3Tobase10("2012010201"));
    assert(SymmetricPattern[10][base3Tobase10("1021020122")] == base3Tobase10("1021020122"));
    assert(SymmetricPattern[10][base3Tobase10("2122212222")] == base3Tobase10("2122212222"));
    assert(SymmetricPattern[10][base3Tobase10("1100001000")] == base3Tobase10("1100001000"));
    assert(SymmetricPattern[10][base3Tobase10("0000000000")] == base3Tobase10("0000000000"));
    assert(SymmetricPattern[10][base3Tobase10("1111111111")] == base3Tobase10("1111111111"));
    assert(SymmetricPattern[10][base3Tobase10("2222222222")] == base3Tobase10("2222222222"));

    // extract
    assert(extract(
               makeBB({
                   0b00000001,
                   0b00000010,
                   0b00000000,
                   0b00000000,
                   0b00000001,
                   0b00000000,
                   0b00000001,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b00000001,
                   0b00000000,
                   0b00000001,
                   0b00000000,
                   0b00000001,
                   0b00000010,
                   0b00000000,
               }),
               0) == base3Tobase10("0212120121"));

    assert(extract(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000001,
                   0b00000011,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000001,
                   0b00000011,
                   0b00000001,
               }),
               0) == base3Tobase10("2222222222"));

    assert(extract(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000010,
                   0b00000010,
                   0b00000010,
                   0b00000010,
                   0b00000010,
                   0b00000010,
                   0b00000010,
                   0b00000010,
               }),
               1) == base3Tobase10("22222222"));

    assert(extract(
               makeBB({
                   0b00000010,
                   0b00000001,
                   0b00000100,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000001,
                   0b00000100,
                   0b00000010,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               }),
               4) == base3Tobase10("012201120"));

    assert(extract(
               makeBB({
                   0b00000000,
                   0b10000000,
                   0b01000000,
                   0b00100000,
                   0b00010000,
                   0b00000000,
                   0b00000100,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00001000,
                   0b00000000,
                   0b00000010,
               }),
               6) == base3Tobase10("1111212"));

    assert(extract(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b10000000,
                   0b01000000,
                   0b00100000,
                   0b00010000,
               }),
               9) == base3Tobase10("2222"));

    assert(extract(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000100,
                   0b00000001,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000010,
                   0b00000100,
               }),
               10) == base3Tobase10("0012000201"));
    // getMoves, getFlip

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

        if (moves0 != moves1)
            ++diff_getmoves;

        int chosen = mt64() % popcount(blank);
        while (chosen--)
            blank &= blank - 1ULL;

        Bitboard sqbit = blank & -blank;

        Bitboard flip0 = getFlip(b, w, sqbit);
        Bitboard flip1 = getFlip_slow(b, w, sqbit);

        if (flip0 != flip1) {
            ++diff_getflip;

            printBitboard(flip0);
            printBitboard(flip1);
        }
    }

    std::cout << diff_getmoves << std::endl
              << diff_getflip << std::endl;
}

// assert(
//     makeBB({
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//     }) ==
//     rotateRightBy90(makeBB({
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//         0b00000000,
//     })));

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
                sq = chooseRandomMove(reversi.p, reversi.o, mt);
            } else {
                sq = chooseBestMove(reversi.p, reversi.o, depth);
            }
            reversi.move(sq);
        }

        if (reversi.winner() == ~randomPlayer)
            ++wins;
    }

    std::cout << "Win rate:" << ((double)wins / (double)N * 100) << "%" << std::endl;
}