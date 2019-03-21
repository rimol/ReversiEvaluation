#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <time.h>
#include "bitboard.h"
#include "recode.h"
#include "evalgen.h"

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
static inline Feature convert(Bitboard b, Bitboard w) {
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

static inline Feature extractHorizontal(int n, Bitboard p, Bitboard o) {
    int shift = (7 - n) * 8;
    return convert(p >> shift, o >> shift);
}

static inline Feature extractVertical(int n, Bitboard p, Bitboard o) {
    p = rotateRightBy90(p);
    o = rotateRightBy90(o);
    int shift = (7 - n) * 8;
    return convert(p >> shift, o >> shift);
}

static inline Feature extractCorner(int n, Bitboard p, Bitboard o) {
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

static int horizontal[8][6561];
static int vertical[8][6561];
static int corner[4][6561];
static int mobility = 0;

static int evaluate(Bitboard p, Bitboard o) {
    int e = 0;

    // ↓とても汚い

    // horver
    for (int i = 0; i < 8; ++i) {
        e += horizontal[i][extractHorizontal(i, p, o)];
        e += vertical[i][extractVertical(i, p, o)];
    }
    // corner
    for (int i = 0; i < 4; ++i) {
        e += corner[i][extractCorner(i, p, o)];
    }

    e += getMobility(p, o) * mobility;

    return e;
}

// 更新分の保存用
static int horDiff[8][6561];
static int verDiff[8][6561];
static int corDiff[4][6561];
static int mobDiff = 0;

// 結果はhorizontal, vertical, ...の配列に書き込む
// 評価値は最終石差*1000の近似とする
static void calculateEvaluationValue(std::string recodeFilePath) {
    // 更新の繰り返し回数
    constexpr int N = 100;
    // 配列の初期化（0埋め）
    // diffの方は0毎回0になるはずですが一応
    std::fill((int*)horizontal, (int*)(horizontal + 8), 0);
    std::fill((int*)vertical, (int*)(vertical + 8), 0);
    std::fill((int*)corner, (int*)(corner + 4), 0);    
    mobility = 0;
    std::fill((int*)horDiff, (int*)(horDiff + 8), 0);
    std::fill((int*)verDiff, (int*)(verDiff + 8), 0);
    std::fill((int*)corDiff, (int*)(corDiff + 4), 0);    
    mobDiff = 0;
    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::ifstream ifs(recodeFilePath, std::ios::ate | std::ios::binary);
    // ファイルに入っている局面の数
    const int M = ifs.tellg() / sizeof(Recode);
    std::vector<Recode> recodes(M);
    //beginから0バイトのところにストリーム位置を変更
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*)&recodes[0], M * sizeof(Recode));
    // N回ループ
    for (int k = 0; k < N; ++k) {
        for (Recode recode : recodes) {
            // 残差
            int r = (recode.result - evaluate(recode.p, recode.o)) * 1000;
            // このデータで出現する各特徴に対し更新分を加算していく
            // horver
            for (int i = 0; i < 8; ++i) {
                // hor[i], ver[i]
                Feature hor_i = extractHorizontal(i, recode.p, recode.o);
                Feature ver_i = extractVertical(i, recode.p, recode.o);
                horDiff[i][hor_i] += r;
                verDiff[i][ver_i] += r;
            }

            // corner
            for (int i = 0; i < 4; ++i) {
                Feature cor_i = extractCorner(i, recode.p, recode.o);
                corDiff[i][cor_i] += r;
            }

            // mobility
            mobDiff += r * getMobility(recode.p, recode.o);
        }

        // 更新分を適用。
        const int W = 1;
        // horver
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 6561; ++j) {
                horizontal[i][j] += horDiff[i][j] * W;
                vertical[i][j] += verDiff[i][j] * W;
                // 掃除もついでにする.
                horDiff[i][j] = verDiff[i][j] = 0;
            }
        }
        // corner
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 6561; ++j) {
                corner[i][j] += corDiff[i][j] * W;
                corDiff[i][j] = 0;
            }
        }
        // mobility
        mobility += mobDiff * W;
        mobDiff = 0;
    }
}

void generateEvaluationFiles(std::string recodesFolderPath, std::string outputFolderPath) {
    // 正しいフォルダ以外が指定されたときのことはめんどくさいので考えません
    std::stringstream ss0;
    std::stringstream ss1;
    ss0 << recodesFolderPath;
    ss1 << outputFolderPath;
    if (recodesFolderPath.back() != '\\') ss0 << '\\';
    if (outputFolderPath.back() != '\\') ss1 << '\\';
    ss1 << time(NULL) << "\\";
    // フォルダ作成
    mkdir(ss1.str().c_str());

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 1; i <= 60; ++i) {
        std::stringstream _ss0;
        _ss0 << ss0.str() << i << ".bin";
        // ファイルパスを渡して計算させる
        calculateEvaluationValue(_ss0.str());
        // 保存～
        std::stringstream _ss1;
        _ss1 << ss1.str() << i << ".bin";
        std::ofstream ofs(_ss1.str(), std::ios::binary);
        ofs.write((char*)horizontal, sizeof(int) * 8 * 6561);
        ofs.write((char*)vertical, sizeof(int) * 8 * 6561);
        ofs.write((char*)corner, sizeof(int) * 4 * 6561);
        ofs.write((char*)&mobility, sizeof(int));
    }
}