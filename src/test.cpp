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
    // parity
    assert(parity(0xffffffffffffffffULL) == -1);
    assert(parity(0xffffffffefffffffULL) == 1);
    assert(parity(0ULL) == -1);
    assert(parity(0x0123456789abcdefULL) == -1);

    // parity sum
    assert(paritySum(0xffffffffffffffffULL) == -4);
    assert(paritySum(0xffffffffefffffffULL) == -2);
    assert(paritySum(0ULL) == -4);
    assert(paritySum(0x8000000000000001ULL) == 0);
    assert(paritySum(0x8800000000000010ULL) == 2);
    assert(paritySum(0x8800000000000011ULL) == 4);
    assert(paritySum(0x0123456789abcdefULL) == -4);

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
    assert(SymmetricPattern[1][base3Tobase10("22222211")] == base3Tobase10("11222222"));
    assert(SymmetricPattern[1][base3Tobase10("11222222")] == base3Tobase10("11222222"));
    assert(SymmetricPattern[1][base3Tobase10("11222211")] == base3Tobase10("11222211"));
    assert(SymmetricPattern[1][base3Tobase10("02121210")] == base3Tobase10("01212120"));
    assert(SymmetricPattern[1][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    assert(SymmetricPattern[1][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    assert(SymmetricPattern[1][base3Tobase10("22222222")] == base3Tobase10("22222222"));

    assert(SymmetricPattern[4][base3Tobase10("21212201")] == base3Tobase10("12202112"));
    assert(SymmetricPattern[4][base3Tobase10("20012012")] == base3Tobase10("20012012"));
    assert(SymmetricPattern[4][base3Tobase10("10101002")] == base3Tobase10("10101002"));
    assert(SymmetricPattern[4][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    assert(SymmetricPattern[4][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    assert(SymmetricPattern[4][base3Tobase10("22222222")] == base3Tobase10("22222222"));

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

    assert(SymmetricPattern[10][base3Tobase10("01101101")] == base3Tobase10("01101101"));
    assert(SymmetricPattern[10][base3Tobase10("21122112")] == base3Tobase10("12211221"));
    assert(SymmetricPattern[10][base3Tobase10("01121201")] == base3Tobase10("01121201"));
    assert(SymmetricPattern[10][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    assert(SymmetricPattern[10][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    assert(SymmetricPattern[10][base3Tobase10("22222222")] == base3Tobase10("22222222"));

    // extract
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
               4) == base3Tobase10("01220120"));

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
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000110,
                   0b00000110,
                   0b00000110,
                   0b00000110,
                   0b00000000,
                   0b00000000,
               }),
               10) == base3Tobase10("22222222"));

    assert(extract(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000100,
                   0b00000010,
                   0b00000010,
                   0b00000010,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000100,
                   0b00000100,
                   0b00000000,
                   0b00000000,
               }),
               10) == base3Tobase10("10012121"));
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
        Color randomPlayer = Black;
        // (Color)(mt() & 1);
        Reversi reversi;
        std::vector<Reversi> pos;
        while (!reversi.isFinished) {
            int sq;
            if (reversi.c == randomPlayer) {
                sq = chooseRandomMove(reversi.p, reversi.o, mt);
            } else {
                sq = chooseBestMove(reversi.p, reversi.o, depth);
            }
            reversi.move(sq);
            pos.push_back(reversi);
        }

        if (reversi.winner() == ~randomPlayer)
            ++wins;
        else {
            for (auto &p : pos) {
                p.print();
            }
        }
    }

    std::cout << "Win rate:" << ((double)wins / (double)N * 100) << "%" << std::endl;
}