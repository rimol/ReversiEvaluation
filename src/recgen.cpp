#include <vector>
#include <random>
#include "bitboard.h"
#include "board.h"
#include "recgen.h"
#include "solver.h"

void generateRecode(int n) {
    std::random_device rnd; // 非決定的乱数生成器、これでメルセンヌ・ツイスタのシードを設定
    std::mt19937 mt(rnd());

    std::vector<Board> boards(60);
    for (int i = 0; i < n; ++i) {
        bool passed = false;
        int blackCount = 0;
        int whiteCount = 0;
        // 終局までの手数
        int turns = 0;
        // 初期
        boards[0] = Board(Black, 0x0000000810000000ULL, 0x0000001008000000ULL);
        
        for (int j = 1; j < 60; ++j) {
            Board current = boards[j - 1];
            int c = (int)current.c;
            Bitboard p = current.bits[c];
            Bitboard o = current.bits[c ^ 1];

            Bitboard moves = getMoves(p, o);
            if (moves == 0ULL) {
                if (passed) {
                    blackCount = popcount(current.bits[Black]);
                    whiteCount = popcount(current.bits[White]);
                    break;
                }
                else {
                    passed = true;
                    boards[j] = Board((Color)(c ^ 1), o, p);
                    continue;
                }
            }
            else {
                // 打つ手をランダムに決める
                int chosen = mt() % (popcount(moves));
                // 下からビットを剥がしていく
                while (chosen--) moves &= moves - 1;
                // 一番下以外のビットを消す
                Bitboard sqbit = -moves & moves;
                Bitboard flip = getFlip(p, o, sqbit);

                p ^= flip | sqbit;
                o ^= flip;

                boards[j] = Board((Color)(c ^ 1), o, p);
            }
        }
    }
}