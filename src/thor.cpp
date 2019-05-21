#include <fstream>
#include "recode.h"
#include "reversi.h"
#include "thor.h"

void convertThorDatabaseToRecodeFiles(const std::vector<std::filesystem::path> &thorDatabasePaths, std::filesystem::path outputFolderPath) {
    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(outputFolderPath / (std::to_string(i + 1) + ".bin"), std::ios::binary);
    }

    for (auto path : thorDatabasePaths) {
        std::ifstream ifs(path.string(), std::ios::ate | std::ios::binary);
        if (!ifs) {
            std::cout << "entered path is invalid" << std::endl;
            continue;
        }
        // databaseに入っている試合数を割り出す
        const int M = ((int)ifs.tellg() - 16) / sizeof(Thor);
        std::vector<Thor> database(M);
        // ヘッダーの16バイトを飛ばす
        ifs.seekg(16);
        ifs.read((char*)&database[0], M * sizeof(Thor));

        for (auto thor : database) {
            int result = thor.blackStoneCount - (64 - thor.blackStoneCount);
            Reversi reversi;
            for (int i = 0; !reversi.isFinished; ++i) {
                int move = thor.moves[i];
                int x = 8 - move % 10;
                int y = 8 - move / 10;
                // 僕のソフトでの形式に変換
                int sq = x + y * 8;
                reversi.move(sq);
                Recode recode(reversi.p, reversi.o, reversi.c, i + 1);
                if (reversi.c == White) {
                    std::swap(recode.board[Black], recode.board[White]);
                    recode.result = -result;
                }
                else {
                    recode.result = result;
                }

                ofss[i].write((char*)&recode, sizeof(Recode));
            }
        }
    }
}