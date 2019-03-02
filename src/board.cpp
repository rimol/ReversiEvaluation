#include <iostream>
#include "board.h"

void printBoard(const Board b) {
    std::cout << (b.c == Black ? "Black" : "White") << std::endl;
    for (int i = 63; i >= 0; --i) {
        Bitboard bit = 1ULL << i;
        if (b.bits[Black] & bit) {
            std::cout << "o";
        }
        else if (b.bits[White] & bit) {
            std::cout << "x";
        }
        else {
            std::cout << "-";
        }

        if (i % 8 == 0) std::cout << std::endl;
    }

    std::cout << std::endl;
}