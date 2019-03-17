#include <vector>
#include <fstream>
#include "bitboard.h"
#include "recode.h"
#include "evalgen.h"

// 下位8ビットの表す盤面を3進数での表現に変換
// 空=0, 黒=1, 白=2;
// いいビット演算が思いつかなかったので素直に実装する
static Feature convert(Bitboard b, Bitboard w) {
    Feature converted = 0U;
    Feature multiple = 1U;
    for (int i = 0; i < 8; ++i) {
        Bitboard t0 = b >> i & 1ULL;
        Bitboard t1 = w >> i & 1ULL;
        converted += (Feature)(t0 | t1 << 1) * multiple;
        multiple *= 3U;
    }
    return converted;
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

static int horizontal[8][6561];
static int vertical[8][6561];
static int corner[8][6561];
static int mobility = 0;

static int horDiff[8][6561];
static int verDiff[8][6561];
static int corDiff[8][6561];

void generateEvaluationData(std::string recodeFilePath) {
    // 更新の繰り返し回数
    constexpr int M = 10000;
    // ファイルを開く
    std::ifstream ifs(recodeFilePath, std::ios::in | std::ios::binary);
    while (!ifs.eof()) {
        Recode recode;
        ifs.read((char*)&recode, sizeof(Recode));
        // 現在の重み係数で計算された評価値
        int _e = 0;
        // hor
        for (int i = 0; i < 8; ++i) {

        }
    }
}