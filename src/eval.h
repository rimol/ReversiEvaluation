#pragma once

#include "bitboard.h"
#include <cassert>
#include <string>

// 一つの特徴で何個のマスまでデータをもつか指定しておく
constexpr int MaxFocusedSquareCount = 10;

constexpr int integerPow(int x, int n) {
    if (n == 1)
        return x;
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
.......# ......#. .....#.. ....#... .....### #....... ........ ........ ........ ........ ........
......## ......#. .....#.. ....#... .....### .#...... #....... ........ ........ ........ ........
.......# ......#. .....#.. ....#... .....### ..#..... .#...... #....... ........ ........ ........
.......# ......#. .....#.. ....#... ........ ...#.... ..#..... .#...... #....... ........ ........
.......# ......#. .....#.. ....#... ........ ....#... ...#.... ..#..... .#...... #....... ........
.......# ......#. .....#.. ....#... ........ .....#.. ....#... ...#.... ..#..... .#...... ........
......## ......#. .....#.. ....#... ........ ......#. .....#.. ....#... ...#.... ..#..... ...#####
.......# ......#. .....#.. ....#... ........ .......# ......#. .....#.. ....#... ...#.... ...#####
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

enum Symmetory {
    R90F,
    R180F,
    F,
    R90,
    R180,
    Asymmetry
};

struct _Feature {
    int featureNum;
    Symmetory featureSymmetory[GroupNum];
    Bitboard masks[GroupNum];

    constexpr _Feature() : featureNum(), masks{}, featureSymmetory{} {
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
        // 左右反転も考える
        for (int i = 0; i < GroupNum; ++i) {
            Bitboard mask0 = masks[i];
            Bitboard mask90 = rotateRightBy90(mask0);
            Bitboard mask180 = rotateBy180(mask0);
            Bitboard mask270 = rotateRightBy90(mask180);
            Bitboard flipped = flipVertical(mask0);

            int memberNum = 0;
            // 90度回転が一致すれば、ほかの回転度のものも全部一致させることができる
            if (mask0 == mask90) {
                featureSymmetory[i] = R90;
                featureNum += (memberNum = 2);
            }
            // 90度回転が一致しないときは, 270度回転も一致しない。（一致するとすればそのまま回転させ続けて90度に一致させることができることになって矛盾）
            else if (mask0 == mask180) {
                featureSymmetory[i] = R180;
                featureNum += (memberNum = 4);
            } else {
                featureSymmetory[i] = Asymmetry;
                featureNum += (memberNum = 8);
            }

            if (flipped == mask0 || flipped == mask90 || flipped == mask180 || flipped == mask270) {
                featureSymmetory[i] = (Symmetory)(featureSymmetory[i] - 3);
                featureNum -= memberNum / 2;
            }
        }
    }
};

constexpr int FeatureNum = _Feature().featureNum;

enum RotationType {
    Rot0,
    Rot90,
    Rot180,
    Rot270,
    Rot0F,
    Rot90F,
    Rot180F,
    Rot270F
};

struct __Feature {
    // 特徴 f がどのmaskを(group)、どれくらい回転・反転したものか(rotationType)を求めておく.
    int group[FeatureNum];
    RotationType rotationType[FeatureNum];

    constexpr __Feature() : group{}, rotationType{} {
        int x = 0;
        constexpr auto feature = _Feature();
        for (int i = 0; i < GroupNum; ++i) {
            switch (feature.featureSymmetory[i]) {
            case R90F:
                group[x] = i;
                rotationType[x] = Rot0;

                x += 1;
                break;

            case R180F:
                group[x] = group[x + 1] = i;
                rotationType[x] = Rot0;
                rotationType[x + 1] = Rot90;

                x += 2;
                break;

            case F:
                group[x] = group[x + 1] = group[x + 2] = group[x + 3] = i;
                rotationType[x] = Rot0;
                rotationType[x + 1] = Rot90;
                rotationType[x + 2] = Rot180;
                rotationType[x + 3] = Rot270;

                x += 4;
                break;

            case R90:
                group[x] = group[x + 1] = i;
                rotationType[x] = Rot0;
                rotationType[x + 1] = Rot0F;

                x += 2;
                break;

            case R180:
                group[x] = group[x + 1] = group[x + 2] = group[x + 3] = i;
                rotationType[x] = Rot0;
                rotationType[x + 1] = Rot90;
                rotationType[x + 2] = Rot0F;
                rotationType[x + 3] = Rot90F;

                x += 4;
                break;

            case Asymmetry:
                for (int j = 0; j < 8; ++j)
                    group[x + j] = i;

                rotationType[x] = Rot0;
                rotationType[x + 1] = Rot90;
                rotationType[x + 2] = Rot180;
                rotationType[x + 3] = Rot270;
                rotationType[x + 4] = Rot0F;
                rotationType[x + 5] = Rot90F;
                rotationType[x + 6] = Rot180F;
                rotationType[x + 7] = Rot270F;

                x += 8;
                break;
            }
        }
    }
};

// ↑のをまとめる
struct ___Feature {
    Bitboard masks[GroupNum];
    int group[FeatureNum];
    RotationType rotationType[FeatureNum];

    constexpr ___Feature() : masks{}, group{}, rotationType{} {
        constexpr auto g = _Feature();
        constexpr auto h = __Feature();

        // g, hの内容をコピーする
        for (int i = 0; i < GroupNum; ++i) {
            masks[i] = g.masks[i];
        }
        for (int i = 0; i < FeatureNum; ++i) {
            group[i] = h.group[i];
            rotationType[i] = h.rotationType[i];
        }
    }
};

constexpr ___Feature Feature = ___Feature();

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

extern int SymmetricPattern[GroupNum][EvalArrayLength];
void initSymmetricPattern();

// abcdef...(2) -> abcdef...(3)
struct Base3Conversion {
    int table[1 << MaxFocusedSquareCount];

    constexpr Base3Conversion() : table{} {
        for (int i = 0; i < (1 << MaxFocusedSquareCount); ++i) {
            int d = 1;
            for (int j = 0; j < MaxFocusedSquareCount; ++j) {
                if (i >> j & 1)
                    table[i] += d;
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
    return SymmetricPattern[g][convert(pext(p, Feature.masks[g]), pext(o, Feature.masks[g]))];
}

void loadEvalValues(std::string evalValuesFolderPath);
double evaluate(Bitboard p, Bitboard o);