#include "emscripten.h"
#include "solver.h"

using HalfBitboard = unsigned int;

extern "C" {

/*
最善スコア、探索ノード数、スコア確定時間、手順まで読み切り時間、
最善手順(60)
 */
static int solutionArray[64];

EMSCRIPTEN_KEEPALIVE
int *solve_exported(HalfBitboard p0, HalfBitboard p1, HalfBitboard o0, HalfBitboard o1) {
    Bitboard p = (Bitboard)p0 << 32 | (Bitboard)p1;
    Bitboard o = (Bitboard)o0 << 32 | (Bitboard)o1;

    auto solution = solve(p, o);
    solutionArray[0] = solution.bestScore;
    solutionArray[1] = (int)solution.nodeCount;
    solutionArray[2] = (int)solution.scoreLockTime;
    solutionArray[3] = (int)solution.wholeTime;
    for (int i = 0; i < 60; ++i) {
        solutionArray[i + 4] = solution.bestMoves[i];
    }

    return &solutionArray[0];
}
}