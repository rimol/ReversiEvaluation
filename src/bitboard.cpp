#include <iostream>
#include "bitboard.h"

void printBitboard(const Bitboard x) {
    for (int i = 63; i >= 0; --i) {
        std::cout << ((x >> i & 1) ? 'o' : '-');
        if (i % 8 != 0) std::cout << ' ';
        else std::cout << std::endl;
    }
}