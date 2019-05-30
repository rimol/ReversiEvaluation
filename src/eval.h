#pragma once

#include <cassert>
#include <filesystem>
#include <string>
#include "bitboard.h"

// 一つの特徴で何個のマスまでデータをもつか指定しておく
constexpr int MaxFocusedSquareCount = 10;

constexpr int integerPow(int x, int n) {
    if (n == 1) return x;
    return n & 1 ? x * integerPow(x * x, n / 2) : integerPow(x * x, n / 2);
}

// もったいないけどこれで固定しておく
constexpr int EvalArrayLength = integerPow(3, MaxFocusedSquareCount);

/*
書式:
改行
72 * GroupNum
ヌル文字
*/
constexpr char FeatureDefinitions[] = 
R"(
######## ........ ........ ........ ###..... .......# ......#. .....#.. ....#... ...#.... #####...
.#....#. ######## ........ ........ ###..... ......#. .....#.. ....#... ...#.... ..#..... #####...
........ ........ ######## ........ ###..... .....#.. ....#... ...#.... ..#..... .#...... ........
........ ........ ........ ######## ........ ....#... ...#.... ..#..... .#...... #....... ........
........ ........ ........ ........ ........ ...#.... ..#..... .#...... #....... ........ ........
........ ........ ........ ........ ........ ..#..... .#...... #....... ........ ........ ........
........ ........ ........ ........ ........ .#...... #....... ........ ........ ........ ........
........ ........ ........ ........ ........ #....... ........ ........ ........ ........ ........
)";

/*
........ ........ ........ ........
........ ........ ........ ........
........ ........ ........ ........
........ ........ ........ ........
........ ........ ........ ........
........ ........ ........ ........
........ ........ ........ ........
........ ........ ........ ........
*/

constexpr int DefinitionLength = sizeof(FeatureDefinitions) / sizeof(char);
constexpr int GroupNum = (DefinitionLength - 2) / 72;

struct _Feature {
    int featureNum;
    int memberNum[GroupNum];
    Bitboard masks[GroupNum];

    constexpr _Feature() : featureNum(), masks{}, memberNum{} {
        int x = 1;
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < GroupNum; ++j) {
                Bitboard bits = 0ULL;
                for (int k = 0; k < 8; ++k) {
                    if (FeatureDefinitions[x + k] == '#') {
                        bits |= 1ULL << (7 - k);
                    }
                }
                x += 9;
                masks[j] |= bits << ((7 - i) * 8);
            }
        }

        // 各特徴について90,180,270回転したマスクを生成して何種類あるか調べていく
        for (int i = 0; i < GroupNum; ++i) {
            Bitboard mask0 = masks[i];
            Bitboard mask90 = rotateRightBy90(mask0);
            Bitboard mask180 = rotateBy180(mask0);

             // 90度回転が一致すれば、ほかの回転度のものも全部一致させることができる
            if (mask0 == mask90) {
                memberNum[i] = 1;
                featureNum += 1;
            }
            // 90度回転が一致しないときは, 270度回転も一致しない。（一致するとすればそのまま回転させ続けて90度に一致させることができることになって矛盾）
            else if (mask0 == mask180) {
                memberNum[i] = 2;
                featureNum += 2;
            }
            else {
                memberNum[i] = 4;
                featureNum += 4;
            }
        }
    }
};

constexpr int FeatureNum = _Feature().featureNum;

struct __Feature {
    int group[FeatureNum];
    int rotationType[FeatureNum];

    constexpr __Feature() : group{}, rotationType{} {
        int x = 0;
        constexpr auto feature = _Feature();
        for (int i = 0; i < GroupNum; ++i) {
            for (int j = 0; j < feature.memberNum[i]; ++j) {
                group[x + j] = i;
                switch (feature.memberNum[i]) {
                    case 1:
                    rotationType[x + j] = 0;
                    break;
                    case 2:
                    rotationType[x + j] = j * 2;
                    break;
                    case 4:
                    rotationType[x + j] = j;
                    break;
                }
            }
            x += feature.memberNum[i];
        }
    }
};

// ↑のをまとめる
struct Features {
    Bitboard masks[GroupNum];
    int group[FeatureNum];
    int rotationType[FeatureNum];

    constexpr Features() : masks{}, group{}, rotationType{} {
        constexpr auto g = _Feature();
        constexpr auto h = __Feature();

        // g, gの内容をコピーする
        for (int i = 0; i < GroupNum; ++i) {
            masks[i] = g.masks[i];
        }
        for (int i = 0; i < FeatureNum; ++i) {
            group[i] = h.group[i];
            rotationType[i] = h.rotationType[i];
        }
    }
};

constexpr Features Feature = Features();

static_assert(popcount(Feature.masks[0]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[1]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[2]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[3]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[4]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[5]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[6]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[7]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[8]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[9]) <= MaxFocusedSquareCount);
static_assert(popcount(Feature.masks[10]) <= MaxFocusedSquareCount);


// abcdef...(2) -> abcdef...(3)
struct Base3Conversion {
    int table[1 << MaxFocusedSquareCount];

    constexpr Base3Conversion() : table{} {
        for (int i = 0; i < (1 << MaxFocusedSquareCount); ++i) {
            int d = 1;
            for (int j = 0; j < MaxFocusedSquareCount; ++j) {
                if (i >> j & 1) table[i] += d;
                d *= 3;
            }
        }
    }
};

constexpr auto ToBase3 = Base3Conversion();

// 下位8ビットの表す盤面を3進数での表現に変換
// 空=0, 黒=1, 白=2;
// テーブル最高。
inline int convert(Bitboard p, Bitboard o) {
    constexpr int mask = (1 << MaxFocusedSquareCount) - 1;
    // 3進数では桁かぶりがないので足し算できる。
    return ToBase3.table[p & mask] + (ToBase3.table[o & mask] << 1);
}

inline int extract(Bitboard p, Bitboard o, int g) {
    return convert(pext(p, Feature.masks[g]), pext(o, Feature.masks[g]));
}

void loadEvalValues(std::filesystem::path evalValuesFolderPath);
double evaluate(Bitboard p, Bitboard o);