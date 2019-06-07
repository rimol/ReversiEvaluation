#include <iostream>
#include <algorithm>
#include "bitboard.h"
#include "reversi.h"

Color Reversi::winner() const {
    int diff = popcount(p) - popcount(o);
    return diff > 0 ? c : ~c;
}

// 盤面を進める。
// 不正な手であればfalseを返す。
bool Reversi::move(int sq) {
    if ((moves >> sq & 1ULL) ^ 1ULL) return false;

    Bitboard sqbit = 1ULL << sq;
    Bitboard flip = getFlip(p, o, sqbit);
    p ^= flip | sqbit;
    o ^= flip;

    moves = getMoves(o, p);
    if (moves != 0ULL) {
        std::swap(p, o);
        c = ~c;
    }
    // パスのとき
    else {
        moves = getMoves(p, o);
        if (moves == 0ULL) {
            // 終局
            isFinished = true;
        }
    }

    return true;
}

void Reversi::print() const {
    char cTable[0b100 + 1];
    cTable[0b000] = ' ';
    cTable[0b001] = 'X';
    cTable[0b010] = 'O';
    cTable[0b100] = '*';

    std::cout << "  A B C D E F G H\n";
    for (int i = 63; i >= 0; --i) {
        if (i % 8 == 7) std::cout << (8 - i / 8);
        unsigned int b = (p >> i & 1U) | ((o >> i & 1U) << 1) | ((moves >> i & 1U) << 2);
        std::cout << ' ' << cTable[b];
        if (i % 8 == 0) std::cout << std::endl;
    }

    int diff = popcount(p) - popcount(o);
    if (c == White) diff *= -1;

    std::cout << "(Black)-(White):" << diff << std::endl;
}

std::string convertToLegibleSQ(int sq) {
    std::stringstream ss;
    int x = sq % 8;
    int y = sq / 8;
    ss << "HGFEDCBA"[x] << (8 - y);
    return ss.str();
}