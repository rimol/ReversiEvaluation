#include <fstream>
#include "recode.h"
#include "reversi.h"
#include "thor.h"
#include "util.h"

void convertThorDatabaseToRecodeFiles(const std::vector<std::string> &thorDatabasePaths, std::string outputFolderPath) {
    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(addFileNameAtEnd(outputFolderPath, std::to_string(i + 1), "bin"), std::ios::binary);
    }

    int totalRecodeNum = 0;

    for (auto path : thorDatabasePaths) {
        std::ifstream ifs(path, std::ios::ate | std::ios::binary);
        if (!ifs) {
            std::cout << "entered path is invalid" << std::endl;
            continue;
        }
        // databaseに入っている試合数を割り出す
        const int M = ((int)ifs.tellg() - 16) / sizeof(Thor);
        std::vector<Thor> database(M);
        totalRecodeNum += M;
        // ヘッダーの16バイトを飛ばす
        ifs.seekg(16);
        ifs.read((char*)&database[0], M * sizeof(Thor));

        for (auto thor : database) {
            int result = thor.blackStoneCount - (64 - thor.blackStoneCount);
            Reversi reversi;
            for (int i = 0; !reversi.isFinished; ++i) {
                // このソフトでの形式に変換
                int move = thor.moves[i];
                int x = 8 - move % 10;
                int y = 8 - move / 10;
                int sq = x + y * 8;
                bool success = reversi.move(sq);
                if (!success) {
                    std::cout << "an illegal move is detected: " << sq << std::endl;
                    reversi.print();
                    break;
                } 

                Recode recode(reversi.p, reversi.o, result);

                // 黒目線なので必要があればひっくり返す。
                if (reversi.c == White) {
                    recode.result *= -1;
                }

                ofss[i].write((char*)&recode, sizeof(Recode));
            }
        }
    }

    std::cout << "Total: " << totalRecodeNum << " recodes were converted!" << std::endl;
}