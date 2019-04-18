#pragma once

#include <string>
#include "bitboard.h"

typedef unsigned int Feature;

// abcdef...(2) -> abcdef...(3)
struct Base3Conversion {
    Feature table[1 << 8];

    constexpr Base3Conversion() : table() {
        for (int i = 0; i < (1 << 8); ++i) {
            Feature d = 1U;
            for (int j = 0; j < 8; ++j) {
                if (i >> j & 1) table[i] += d;
                d *= 3U;
            }
        }
    }
};

constexpr auto ToBase3 = Base3Conversion();

// 下位8ビットの表す盤面を3進数での表現に変換
// 空=0, 黒=1, 白=2;
// テーブル最高。
inline Feature convert(Bitboard b, Bitboard w) {
    // 3進数では桁かぶりがないので足し算できる。
    return ToBase3.table[b & 0xff] + (ToBase3.table[w & 0xff] * 2U);
}

/*
  ver0                 ver7
    A  B  C  D  E  F  G  H
1  63 62 61 60 59 58 57 56 hor0
2  55 54 53 52 51 50 49 48
3  47 46 45 44 43 42 41 40
4  39 38 37 36 35 34 33 32
5  31 30 29 28 27 26 25 24
6  23 22 21 20 19 18 17 16
7  15 14 13 12 11 10 09 08
8  07 06 05 04 03 02 01 00 hor7

*/

/*
cor0                              cor1
    A  B  C  D  E  F  G  H
1  63 62 61         60 59        58 57 56
2  55 54 53         52 51        50 49 48
3  47 46         45 44 43 42        41 40

4        39 38 37 36 35 34 33 32
5        31 30 29 28 27 26 25 24

6  23 22         21 20 19 18        17 16
7  15 14 13         12 11        10 09 08
8  07 06 05         04 03        02 01 00
cor2                              cor3

*/

inline Feature extractHorizontal(int n, Bitboard p, Bitboard o) {
    int shift = (7 - n) * 8;
    return convert(p >> shift, o >> shift);
}

inline Feature extractVertical(int n, Bitboard p, Bitboard o) {
    p = rotateRightBy90(p);
    o = rotateRightBy90(o);
    int shift = (7 - n) * 8;
    return convert(p >> shift, o >> shift);
}

inline Feature extractCorner(int n, Bitboard p, Bitboard o) {
    const int shifts[4][3] = {
        // cor0
        { 46, 53, 61 },
        // cor1
        { 40, 48, 56 },
        // cor2
        { 5, 13, 22 },
        // cor3
        { 0, 8, 16 }
    };
    const Bitboard masks[4][3] = {
        // cor0
        { 0b11ULL, 0b111ULL, 0b111ULL },
        // cor1
        { 0b11ULL, 0b111ULL, 0b111ULL },
        // cor2
        { 0b111ULL, 0b111ULL, 0b11ULL },
        // cor3
        { 0b111ULL, 0b111ULL, 0b11ULL },
    };

    // 下位8ビットに集める
    Bitboard _p = (p >> shifts[n][0] & masks[n][0])
        + (p >> shifts[n][1] & masks[n][1])
        + (p >> shifts[n][2] & masks[n][2]);
    Bitboard _o = (o >> shifts[n][0] & masks[n][0])
        + (o >> shifts[n][1] & masks[n][1])
        + (o >> shifts[n][2] & masks[n][2]);

    return convert(_p, _o);
}

// 評価関数で使うフォルダの指定
extern std::string evalValuesFolderPath;

extern double horizontal[8][6561];
extern double vertical[8][6561];
extern double corner[4][6561];
extern double mobility;

// 上の配列すべてを0初期化する
void clearArrays();
// 盤上にt + 4コ石がある盤面を評価するデータに変更する
void changeEvaluationTables(int t);
double evaluate(Bitboard p, Bitboard o);