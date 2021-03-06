#include "bitboard.h"
#include "engine.h"
#include "eval.h"
#include "pattern.h"
#include "reversi.h"
#include <algorithm>
#include <cassert>
#include <chrono>
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

std::string toBase3Str(int x) {
    std::string res = "";
    while (x != 0) {
        res += std::to_string(x % 3);
        x /= 3;
    }
    std::reverse(res.begin(), res.end());
    return res;
}

void printEachSquare(Bitboard b, Bitboard w) {
    // 上位ビットから出力する（人間が数字に変換しやすいため）
    for (int i = 7; i >= 0; --i) {
        for (int j = 7; j >= 0; --j) {
            int sq = i * 8 + j;

            if (b >> sq & 1) {
                std::cout << "1 ";
            } else if (w >> sq & 1) {
                std::cout << "2 ";
            } else {
                std::cout << "0 ";
            }
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
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
    // assert(SymmetricPattern[0][base3Tobase10("2221011111")] == base3Tobase10("1111101222"));
    // assert(SymmetricPattern[0][base3Tobase10("2211011101")] == base3Tobase10("1101101212"));
    // assert(SymmetricPattern[0][base3Tobase10("0122112120")] == base3Tobase10("0122112120"));
    // assert(SymmetricPattern[0][base3Tobase10("0211111101")] == base3Tobase10("0211111101"));
    // assert(SymmetricPattern[0][base3Tobase10("0000000000")] == base3Tobase10("0000000000"));
    // assert(SymmetricPattern[0][base3Tobase10("1111111111")] == base3Tobase10("1111111111"));
    // assert(SymmetricPattern[0][base3Tobase10("2222222222")] == base3Tobase10("2222222222"));

    // assert(SymmetricPattern[1][base3Tobase10("22222211")] == base3Tobase10("11222222"));
    // assert(SymmetricPattern[1][base3Tobase10("11222222")] == base3Tobase10("11222222"));
    // assert(SymmetricPattern[1][base3Tobase10("11222211")] == base3Tobase10("11222211"));
    // assert(SymmetricPattern[1][base3Tobase10("02121210")] == base3Tobase10("01212120"));
    // assert(SymmetricPattern[1][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    // assert(SymmetricPattern[1][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    // assert(SymmetricPattern[1][base3Tobase10("22222222")] == base3Tobase10("22222222"));

    // assert(SymmetricPattern[4][base3Tobase10("212122201")] == base3Tobase10("122021212"));
    // assert(SymmetricPattern[4][base3Tobase10("200120112")] == base3Tobase10("200120112"));
    // assert(SymmetricPattern[4][base3Tobase10("101010102")] == base3Tobase10("101010102"));
    // assert(SymmetricPattern[4][base3Tobase10("000000000")] == base3Tobase10("000000000"));
    // assert(SymmetricPattern[4][base3Tobase10("111111111")] == base3Tobase10("111111111"));
    // assert(SymmetricPattern[4][base3Tobase10("222222222")] == base3Tobase10("222222222"));

    // assert(SymmetricPattern[5][base3Tobase10("22221111")] == base3Tobase10("11112222"));
    // assert(SymmetricPattern[5][base3Tobase10("11112222")] == base3Tobase10("11112222"));
    // assert(SymmetricPattern[5][base3Tobase10("10222201")] == base3Tobase10("10222201"));
    // assert(SymmetricPattern[5][base3Tobase10("02121210")] == base3Tobase10("01212120"));
    // assert(SymmetricPattern[5][base3Tobase10("00000000")] == base3Tobase10("00000000"));
    // assert(SymmetricPattern[5][base3Tobase10("11111111")] == base3Tobase10("11111111"));
    // assert(SymmetricPattern[5][base3Tobase10("22222222")] == base3Tobase10("22222222"));

    // assert(SymmetricPattern[9][base3Tobase10("2120")] == base3Tobase10("0212"));
    // assert(SymmetricPattern[9][base3Tobase10("1202")] == base3Tobase10("1202"));
    // assert(SymmetricPattern[9][base3Tobase10("2002")] == base3Tobase10("2002"));
    // assert(SymmetricPattern[9][base3Tobase10("0210")] == base3Tobase10("0120"));
    // assert(SymmetricPattern[9][base3Tobase10("0000")] == base3Tobase10("0000"));
    // assert(SymmetricPattern[9][base3Tobase10("1111")] == base3Tobase10("1111"));
    // assert(SymmetricPattern[9][base3Tobase10("2222")] == base3Tobase10("2222"));

    // assert(SymmetricPattern[10][base3Tobase10("2012010201")] == base3Tobase10("2012010201"));
    // assert(SymmetricPattern[10][base3Tobase10("1021020122")] == base3Tobase10("1021020122"));
    // assert(SymmetricPattern[10][base3Tobase10("2122212222")] == base3Tobase10("2122212222"));
    // assert(SymmetricPattern[10][base3Tobase10("1100001000")] == base3Tobase10("1100001000"));
    // assert(SymmetricPattern[10][base3Tobase10("0000000000")] == base3Tobase10("0000000000"));
    // assert(SymmetricPattern[10][base3Tobase10("1111111111")] == base3Tobase10("1111111111"));
    // assert(SymmetricPattern[10][base3Tobase10("2222222222")] == base3Tobase10("2222222222"));

    // extract
    assert(Patterns.at("logistello").extract(makeBB({
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

    assert(Patterns.at("logistello").extract(makeBB({
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

    assert(Patterns.at("logistello").extract(makeBB({
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

    assert(Patterns.at("logistello").extract(makeBB({
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

    assert(Patterns.at("logistello").extract(makeBB({
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

    assert(Patterns.at("logistello").extract(makeBB({
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

    assert(Patterns.at("logistello").extract(makeBB({
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

    // 確定石
    assert(getStableStones(
               makeBB({
                   0b11100111,
                   0b11000011,
                   0b10000001,
                   0b00000000,
                   0b00000000,
                   0b10000001,
                   0b11000011,
                   0b11100111,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               })) ==

           makeBB({
               0b11100111,
               0b11000011,
               0b10000001,
               0b00000000,
               0b00000000,
               0b10000001,
               0b11000011,
               0b11100111,
           }));

    assert(getStableStones(
               makeBB({
                   0b00000001,
                   0b00000011,
                   0b00000111,
                   0b00001111,
                   0b00011111,
                   0b00111111,
                   0b01111111,
                   0b11111111,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               })) ==

           makeBB({
               0b00000001,
               0b00000011,
               0b00000111,
               0b00001111,
               0b00011111,
               0b00111111,
               0b01111111,
               0b11111111,
           }));

    assert(getStableStones(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b01111000,
                   0b00111100,
                   0b01000110,
                   0b01011100,
                   0b00111100,
                   0b01111110,
               }),
               makeBB({
                   0b00000000,
                   0b10000000,
                   0b10000000,
                   0b11000000,
                   0b10111000,
                   0b10100000,
                   0b11000000,
                   0b10000000,
               })) ==

           makeBB({
               0b00000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
           }));

    assert(getStableStones(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b01111000,
                   0b00111100,
                   0b01000110,
                   0b01011100,
                   0b00111100,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b10000000,
                   0b10000000,
                   0b11000000,
                   0b10111000,
                   0b10100000,
                   0b11000000,
                   0b11111111,
               })) ==

           makeBB({
               0b00000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b11000000,
               0b11111111,
           }));

    assert(getStableStones(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b01111000,
                   0b00111100,
                   0b01000110,
                   0b01011100,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b10000000,
                   0b10000000,
                   0b11000000,
                   0b10111000,
                   0b10100000,
                   0b11111110,
                   0b11111111,
               })) ==

           makeBB({
               0b00000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10000000,
               0b11111110,
               0b11111111,
           }));

    assert(getStableStones(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00001000,
                   0b00001000,
                   0b00001100,
                   0b00000100,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b10000000,
                   0b11110000,
                   0b11110000,
                   0b11110000,
                   0b11111000,
                   0b11111000,
                   0b11111000,
               })) ==

           makeBB({
               0b00000000,
               0b10000000,
               0b11000000,
               0b11100000,
               0b11110000,
               0b11111000,
               0b11111000,
               0b11111000,
           }));

    assert(getStableStones(
               makeBB({
                   0b10101000,
                   0b01110000,
                   0b11111111,
                   0b01110000,
                   0b10101000,
                   0b00100100,
                   0b00100010,
                   0b00100001,
               }),
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
               })) ==

           makeBB({
               0b10000000,
               0b0000000,
               0b00100000,
               0b00000000,
               0b00000000,
               0b00000000,
               0b00000000,
               0b00000001,
           }));

    assert(getStableStones(
               makeBB({
                   0b10101001,
                   0b00000000,
                   0b00000001,
                   0b10000000,
                   0b00000001,
                   0b10000000,
                   0b00000000,
                   0b10010101,
               }),
               makeBB({
                   0b01010010,
                   0b10000001,
                   0b00000000,
                   0b00000001,
                   0b10000000,
                   0b00000000,
                   0b10000001,
                   0b01001010,
               })) ==

           makeBB({
               0b11110001,
               0b00000001,
               0b00000001,
               0b00000001,
               0b10000000,
               0b10000000,
               0b10000000,
               0b10001111,
           }));

    // WeightedMobility
    // 角の分が加算されているかどうかみるだけ。
    assert(getWeightedMobility(
               makeBB({
                   0b00000000,
                   0b00000000,
                   0b00100100,
                   0b00000000,
                   0b00000000,
                   0b00100100,
                   0b00000000,
                   0b00000000,
               }),
               makeBB({
                   0b00000000,
                   0b01000010,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b00000000,
                   0b01000010,
                   0b00000000,
               })) == 8);

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

        Bitboard rotatedBlack[8], rotatedWhite[8];
        rotateAndFlipBB(b, rotatedBlack);
        rotateAndFlipBB(w, rotatedWhite);

        // for (int i = 0; i < 8; ++i) {
        //     printEachSquare(rotatedBlack[i], rotatedWhite[i]);
        // }

        // for (int j = 0; j < GroupNum; ++j) {
        //     std::cout << j << ": " << toBase3Str(extract(b, w, j)) << std::endl;
        // }

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
void play(int N, int depth, const std::string &weightFolderpath) {
    RandomEngine randomEngine;

    PatternEvaluator evaluator(weightFolderpath);
    NegaScoutEngine negaScoutEngine(evaluator);

    int wins = 0;

    auto start = std::chrono::system_clock::now();

    for (int i = 0; i < N; ++i) {
        Color randomPlayer = White;
        // (Color)(mt() & 1);
        Reversi reversi;
        std::vector<Reversi> pos;
        while (!reversi.isFinished) {
            int sq;
            // evalAllMoves(reversi.p, reversi.o, depth);
            if (reversi.c == randomPlayer) {
                sq = randomEngine.chooseMove(reversi.p, reversi.o, 0);
            } else {
                sq = negaScoutEngine.chooseMove(reversi.p, reversi.o, depth);
            }
            reversi.move(sq);
            pos.push_back(reversi);
        }

        if (reversi.winner() == ~randomPlayer)
            ++wins;
        else {
            pos.back().print();
            // for (auto &board : pos) {
            //     board.print();
            //     std::cout << "eval: " << evaluate(board.p, board.o) << std::endl;
            // }
        }
    }

    auto end = std::chrono::system_clock::now();
    auto elapsed_msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Elapsed: " << elapsed_msec << " msec" << std::endl;
    std::cout << "Win rate:" << ((double)wins / (double)N * 100) << "%" << std::endl;
}

void runSelfPlay(int n, const std::string &weightFolderpath1, const std::string &weightFolderpath2) {
    PatternEvaluator evaluator1(weightFolderpath1);
    PatternEvaluator evaluator2(weightFolderpath2);
    NegaScoutEngine engine1(evaluator1);
    NegaScoutEngine engine2(evaluator2);
    RandomEngine randomEngine;

    std::random_device rnd;
    std::mt19937 mt(rnd());

    int win1 = 0;
    int draw = 0;

    for (int i = 0; i < n; ++i) {
        int randomDepth = mt() % 10;
        Color color1 = (Color)(mt() % 2);
        Reversi reversi;
        while (!reversi.isFinished) {
            int sq;
            if ((reversi.stoneCount() - 4) < randomDepth) {
                sq = randomEngine.chooseMove(reversi.p, reversi.o, 0);
            } else if (reversi.c == color1) {
                sq = engine1.chooseMove(reversi.p, reversi.o, 1);
            } else {
                sq = engine2.chooseMove(reversi.p, reversi.o, 1);
            }

            reversi.move(sq);
        }

        int diff = popcount(reversi.p) - popcount(reversi.o);

        if (diff > 0) {
            if (reversi.c == color1)
                ++win1;
        } else if (diff < 0) {
            if (reversi.c != color1)
                ++win1;
        } else
            ++draw;
    }

    std::cout << "Win(Engine1): " << win1 << std::endl;
    std::cout << "Draw: " << draw << std::endl;
    std::cout << "Lose(Engine1): " << (n - win1 - draw) << std::endl;
}