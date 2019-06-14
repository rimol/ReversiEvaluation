#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include "engine.h"
#include "bitboard.h"
#include "recgen.h"
#include "record.h"
#include "reversi.h"
#include "solver.h"
#include "util.h"

struct RecgenPosition {
    Bitboard p, o;
    Color c;
};

void generateRecord(int n, int depth, std::string folderPath) {
    // 指定されたフォルダに保存用フォルダを作成
    // フォルダ名は現在時刻
    folderPath = createCurrentTimeFolderIn(folderPath);

    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(addFileNameAtEnd(folderPath, std::to_string(i + 1), "bin"), std::ios::binary);
    }

    std::random_device rnd; // 非決定的乱数生成器、これでメルセンヌ・ツイスタのシードを設定
    std::mt19937 mt(rnd());

    // n回自動対戦をさせる
    for (int i = 0; i < n; ++i) {
        Reversi reversi;
        bool solved = false;
        bool errorOccured = false;
        Solution solution;
        Color solverColor;
        std::vector<RecgenPosition> moves;

        while (!errorOccured && !reversi.isFinished) {
            if (64 - reversi.stoneCount() >= depth) {
                solution = solve(reversi.p, reversi.o);
                solverColor = reversi.c;
                solved = true;
            }

            int sq;
            if (solved) {
                sq = solution.bestMoves.front();
                solution.bestMoves.erase(solution.bestMoves.begin());
            }
            else {
                sq = chooseRandomMove(reversi.p, reversi.o, mt);
            }

            errorOccured = !reversi.move(sq);
            if (errorOccured) {
                std::cout << "an illegal move was detected. skip this game." << std::endl;
            }
            else {
                moves.push_back({reversi.p, reversi.o, reversi.c});
            }
        }

        if (errorOccured) continue;

        for (int i = 0; i < 60; ++i) {
            Record record(moves[i].p, moves[i].o, solution.bestScore);
            
            if (moves[i].c != solverColor) record.result *= -1;

            ofss[i].write((char*)&record, sizeof(Record));
        }
    }
}