#include "recgen.h"
#include "bitboard.h"
#include "engine.h"
#include "reversi.h"
#include "solver.h"
#include "util.h"
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

void generateRecords(int n, int randomDepth, int searchDepth, int exactDepth, const std::string &saveFolderpath) {
    auto currentTime = time(nullptr);
    auto local = localtime(&currentTime);
    std::stringstream ss;
    ss << (local->tm_year + 1900) << "年"
       << (local->tm_mon + 1) << "月"
       << local->tm_mday << "日"
       << local->tm_hour << "時"
       << local->tm_min << "分"
       << local->tm_sec << "秒-"
       << "rand" << randomDepth
       << "mid" << searchDepth
       << "exact" << exactDepth;
    // バイナリではなくテキストで書き込む.
    std::ofstream ofs(addFileNameAtEnd(saveFolderpath, ss.str(), "txt"));

    std::random_device rnd;
    std::mt19937 mt(rnd());

    for (int i = 0; i < n; ++i) {
        Reversi reversi;
        while (!reversi.isFinished) {
            int sq;
            if ((reversi.stoneCount() - 4) < randomDepth) {
                sq = chooseRandomMove(reversi.p, reversi.o, mt);
            } else if ((reversi.stoneCount() + exactDepth) < 64) {
                sq = chooseBestMove(reversi.p, reversi.o, searchDepth);
            } else {
                break;
            }

            if (reversi.move(sq)) {
                ofs << convertToLegibleSQ(sq);
            } else
                assert(false);
        }

        if (!reversi.isFinished) {
            auto solution = solve(reversi.p, reversi.o);
            for (auto sq : solution.bestMoves) {
                ofs << convertToLegibleSQ(sq);
            }
        }

        ofs << std::endl;
    }
}