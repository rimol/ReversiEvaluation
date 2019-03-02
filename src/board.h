#pragma once
// ビットボードを2つにまとめます, 手番もつけます.
#include "bitboard.h"

enum Color { Black, White };

constexpr Color operator ~(Color c) { return (Color)(c ^ 1); }

struct Board {
    Color c;
    Bitboard bits[2];

    Board() {}
    Board(Color _c, Bitboard b, Bitboard w) : c(_c), bits({b, w}) {}
};

void printBoard(const Board b);