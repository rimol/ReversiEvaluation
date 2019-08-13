#include "bitboard.h"
#include "eval.h"
#include <random>

struct MoveWithScore {
    int move;
    double score;
};

// ソートはされていない
std::vector<MoveWithScore> evalAllMoves(Bitboard p, Bitboard o, int depth);

// 最も良い手（と思われる）ものを返す。
// 返り値は石を置くマス番号。(bitboard.h参照、0<=sq<=63)
int chooseBestMove(Bitboard p, Bitboard o, int depth);
int chooseRandomMove(Bitboard p, Bitboard o, std::mt19937 &mt);