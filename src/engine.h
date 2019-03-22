#include "bitboard.h"
#include "eval.h"
// 最も良い手（と思われる）ものを返す。
// 返り値は石を置くマス番号。(bitboard.h参照、0<=sq<=63)
int chooseBestMove(Bitboard p, Bitboard o);