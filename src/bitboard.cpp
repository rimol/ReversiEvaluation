#include "bitboard.h"

std::ostream& operator << (std::ostream& os, const Bitboard x) {
    for (int i = 63; i >= 0; --i) {
        os << ((x >> i & 1) ? 'o' : '-');
        if (i % 8 != 0) os << ' ';
        else os << std::endl;
    }
    // こうすることで連続的に<<できる
    return os;
}