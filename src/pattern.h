#pragma once

#include "bitboard.h"
#include <map>

// 1つのパターンで最大何個マスをみるか
constexpr int MaxDigit = 10;

constexpr int pow3(int n) {
    int r = 1;
    for (int i = 0; i < n; ++i)
        r *= 3;
    return r;
}

constexpr int MaxNumPattern = pow3(MaxDigit);

// abcdef...(2) -> abcdef...(3)
extern int ToBase3[1 << MaxDigit];
void initToBase3();

// 下位8ビットの表す盤面を3進数での表現に変換
// 空=0, 黒=1, 白=2;
// テーブル最高。
inline int convert(Bitboard p, Bitboard o) {
    constexpr int mask = (1 << MaxDigit) - 1;
    // 3進数では桁かぶりがないので足し算できる。
    return ToBase3[p & mask] + (ToBase3[o & mask] * 2);
}

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

class Pattern {
private:
    int **symmetricPattern;
    int _numGroup;
    int _numPattern;
    int *_group;
    RotationType *_rotationType;
    Bitboard *_mask;

public:
    inline int numGroup() const { return _numGroup; }
    inline int numPattern() const { return _numPattern; }

    inline int group(int pattern) const {
        return _group[pattern];
    }

    inline int rotationType(int pattern) const {
        return _rotationType[pattern];
    }

    inline int extract(Bitboard p, Bitboard o, int group) const {
        return symmetricPattern[group][convert(pext(p, _mask[group]), pext(o, _mask[group]))];
    }

    Pattern &operator=(const Pattern &pattern);

    Pattern();
    Pattern(const Pattern &pattern);
    /*
        patternDefStrの書式:
        改行
        72 * numGroup
        (ヌル文字)
    */
    Pattern(const std::string &patternDefStr);
    ~Pattern();
};

extern const std::map<std::string, Pattern> Patterns;