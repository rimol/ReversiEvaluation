#pragma once

#include <iostream>
#include <immintrin.h>
#include <stdint.h>

enum Color { Black, White };
constexpr Color operator ~(Color c) { return (Color)(c ^ 1); }
std::istream& operator >> (std::istream& is, Color& c);

using Bitboard = unsigned long long;

// CPU命令は使わずに実装する.

/*
ビットと実際のマスの対応

    A  B  C  D  E  F  G  H
1  63 62 61 60 59 58 57 56
2  55 54 53 52 51 50 49 48
3  47 46 45 44 43 42 41 40
4  39 38 37 36 35 34 33 32
5  31 30 29 28 27 26 25 24
6  23 22 21 20 19 18 17 16
7  15 14 13 12 11 10 09 08
8  07 06 05 04 03 02 01 00 <- 0ビット目

*/

// 立っているビットの数を数える
constexpr Bitboard popcount(Bitboard x) {
    // x -= (x >> 1) & 0x5555555555555555ULL;
    // x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    // x = (x & 0x0f0f0f0f0f0f0f0fULL) + ((x >> 4) & 0x0f0f0f0f0f0f0f0fULL);
    // return x * 0x0101010101010101ULL >> 56;
    return __builtin_popcountll(x);
}

// 一番下のビットが下から数えて何ビット目にあるか求める(0-indexed)
inline Bitboard tzcnt(Bitboard x) {
    // return popcount(~x & x - 1ULL);
    return __builtin_ctzll(x);
}

inline Bitboard pext(Bitboard x, Bitboard mask) {
    /*
    maskで立っているビットの数は8に限定しています
    Bitboard extracted = 0ULL;
    for (Bitboard i = 1ULL; i < (1ULL << MaxFocusedSquareCount); i <<= 1) {
        Bitboard lb = mask & -mask;
        if (x & lb) extracted |= i;
        mask ^= lb;
    }
    return extracted;
    */
    return _pext_u64(x, mask);
}

// コピペ
constexpr Bitboard delta_swap(Bitboard x, Bitboard mask, int delta) {
    // ペアのxor
    Bitboard t = (x ^ (x >> delta)) & mask;
    // これを使えば簡単にswapできる
    return t ^ (t << delta) ^ x;
}

// ビット列を逆転する。 ex) 1010 -> 0101
constexpr Bitboard rotateBy180(Bitboard x) {
    x = (x >> 32) | (x << 32);
    x = (x >> 16 & 0x0000ffff0000ffffULL) | (x << 16 & 0xffff0000ffff0000ULL);
    x = (x >> 8 & 0x00ff00ff00ff00ffULL) | (x << 8 & 0xff00ff00ff00ff00ULL);
    x = (x >> 4 & 0x0f0f0f0f0f0f0f0fULL) | (x << 4 & 0xf0f0f0f0f0f0f0f0ULL);
    x = (x >> 2 & 0x3333333333333333ULL) | (x << 2 & 0xccccccccccccccccULL);
    return (x >> 1 & 0x5555555555555555ULL) | (x << 1 & 0xaaaaaaaaaaaaaaaaULL);
} 

// ビットボードを右に90°回転させる.
// 対角線A8H1を軸に盤面を反転させて、1, 2, 3, ..., 8列を反転させて8, 7, 6, ..., 1列にすればよい
constexpr Bitboard rotateRightBy90(Bitboard x) {
    /*
    0x000000000f0f0f0f = 
    0000 0000
    0000 0000
    0000 0000
    0000 0000

    0000 1111
    0000 1111
    0000 1111
    0000 1111
    */
    x = delta_swap(x, 0x000000000f0f0f0fULL, 36);
    /*
    0x0000333300003333 = 
    0000 0000
    0000 0000
    0011 0011
    0011 0011

    0000 0000
    0000 0000
    0011 0011
    0011 0011
    */
    x = delta_swap(x, 0x0000333300003333ULL, 18);
    /*
    0x0055005500550055 = 
    0000 0000
    0101 0101
    0000 0000
    0101 0101

    0000 0000
    0101 0101
    0000 0000
    0101 0101
    */
   x = delta_swap(x, 0x0055005500550055ULL, 9);
   // 横列反転
   x = x >> 32 | x << 32;
   x = (x >> 16 & 0x0000ffff0000ffffULL) | (x & 0x0000ffff0000ffffULL) << 16;
   return (x >> 8 & 0x00ff00ff00ff00ffULL) | (x & 0x00ff00ff00ff00ffULL) << 8;
}

// 打てるマスのビットを立てる
inline Bitboard getMoves(Bitboard p, Bitboard o) {
    Bitboard moves = 0ULL;
    Bitboard blank = ~(p | o);
    // マスク済み敵石のビットボード
    Bitboard mo = o & 0x7e7e7e7e7e7e7e7eULL;
    Bitboard ps = p << 1;
    // 一時変数
    Bitboard t = 0ULL;

    // 右
    moves = (mo + ps) & blank & ~ps;
    // 左
    t = p;
    t |= t >> 1 & mo;
    mo &= mo >> 1;
    t |= t >> 2 & mo;
    mo &= mo >> 2;
    t |= t >> 4 & mo;
    moves |= (t ^ p) >> 1 & blank;
    // 下
    mo = o & 0x00ffffffffffff00ULL;
    t = p;
    t |= t << 8 & mo;
    mo &= mo << 8;
    t |= t << 16 & mo;
    mo &= mo << 16;
    t |= t << 32 & mo;
    moves |= (t ^ p) << 8 & blank;
    // 上
    mo = o & 0x00ffffffffffff00ULL;
    t = p;
    t |= t >> 8 & mo;
    mo &= mo >> 8;
    t |= t >> 16 & mo;
    mo &= mo >> 16;
    t |= t >> 32 & mo;
    moves |= (t ^ p) >> 8 & blank;
    // 右下
    mo = o & 0x007e7e7e7e7e7e00ULL;
    t = p;
    t |= t << 9 & mo;
    mo &= mo << 9;
    t |= t << 18 & mo;
    mo &= mo << 18;
    t |= t << 36 & mo;
    moves |= (t ^ p) << 9 & blank;
    // 左上
    mo = o & 0x007e7e7e7e7e7e00ULL;
    t = p;
    t |= t >> 9 & mo;
    mo &= mo >> 9;
    t |= t >> 18 & mo;
    mo &= mo >> 18;
    t |= t >> 36 & mo;
    moves |= (t ^ p) >> 9 & blank;
    // 左下
    mo = o & 0x007e7e7e7e7e7e00ULL;
    t = p;
    t |= t << 7 & mo;
    mo &= mo << 7;
    t |= t << 14 & mo;
    mo &= mo << 14;
    t |= t << 28 & mo;
    moves |= (t ^ p) << 7 & blank;
    // 右上
    mo = o & 0x007e7e7e7e7e7e00ULL;
    t = p;
    t |= t >> 7 & mo;
    mo &= mo >> 7;
    t |= t >> 14 & mo;
    mo &= mo >> 14;
    t |= t >> 28 & mo;
    moves |= (t ^ p) >> 7 & blank;

    return moves;
}

// ひっくり返す石があるマスのビットを立てる
inline Bitboard getFlip(Bitboard p, Bitboard o, Bitboard sqbit) {
    Bitboard flip = 0ULL;
    Bitboard mo = o & 0x7e7e7e7e7e7e7e7eULL;

    // 左
    Bitboard d = 0x00000000000000feULL * sqbit;
    Bitboard t = (mo | ~d) + 1ULL & d & p;
    flip = t - (t != 0ULL) & d;
    // 左上
    d = 0x8040201008040200ULL * sqbit;
    t = (mo | ~d) + 1ULL & d & p;
    flip |= t - (t != 0ULL) & d;
    // 上 マスクは付けてはだめ。
    d = 0x0101010101010100ULL * sqbit;
    t = (o | ~d) + 1ULL & d & p;
    flip |= t - (t != 0ULL) & d;
    // 右上
    d = 0x0002040810204080ULL * sqbit;
    t = (mo | ~d) + 1ULL & d & p;
    flip |= t - (t != 0ULL) & d;
    // 右
    t = sqbit;
    t |= t >> 1 & mo;
    mo &= mo >> 1;
    t |= t >> 2 & mo;
    mo &= mo >> 2;
    t |= t >> 4 & mo;
    t >>= 1;
    flip |= t & (-(t & p) << 1);
    // 下
    mo = o & 0x00ffffffffffff00ULL;
    t = sqbit;
    t |= t >> 8 & mo;
    mo &= mo >> 8;
    t |= t >> 16 & mo;
    mo &= mo >> 16;
    t |= t >> 32 & mo;
    t >>= 8;
    flip |= t & (-(t & p) << 8);
    // 右下
    mo = o & 0x007e7e7e7e7e7e00ULL;
    t = sqbit;
    t |= t >> 9 & mo;
    mo &= mo >> 9;
    t |= t >> 18 & mo;
    mo &= mo >> 18;
    t |= t >> 36 & mo;
    t >>= 9;
    flip |= t & (-(t & p) << 9);
    // 左下
    mo = o & 0x007e7e7e7e7e7e00ULL;
    t = sqbit;
    t |= t >> 7 & mo;
    mo &= mo >> 7;
    t |= t >> 14 & mo;
    mo &= mo >> 14;
    t |= t >> 28 & mo;
    t >>= 7;
    flip |= t & (-(t & p) << 7);

    return flip;
}

inline int getMobility(Bitboard p, Bitboard o) {
    return popcount(getMoves(p, o));
}

// 愚直実装ばーじょん
// バグつぶし用

Bitboard getMoves_slow(Bitboard p, Bitboard o);
Bitboard getFlip_slow(Bitboard p, Bitboard o, Bitboard sqbit);

// cout でビットボードを見やすく出力する用
// ostream: coutとか.
// std::ostream& operator << (std::ostream& os, const Bitboard x);
// ↑typedefでunsigned long long の名前を Bitboard に変えているだけなので, もとのオーバーロードと判断がつかないため使えない

void printBitboard(const Bitboard x);