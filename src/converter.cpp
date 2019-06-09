#include <cassert>
#include <fstream>
#include "recode.h"
#include "reversi.h"
#include "converter.h"
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

void getContentsOfNext(std::string tagName, const std::string& gameResultString, int& cur, std::string& out) {
    for (int i = cur; cur < (gameResultString.size() - tagName.size() - 2); ++i) {
        bool found = true;
        for (int j = 0; j < tagName.size(); ++j) {
            if (gameResultString[i + j] != tagName[j]) {
                found = false;
                break;
            }
        }

        if (found && gameResultString[i + tagName.size()] == '[') {
            int left = i + tagName.size() + 1;
            int right = gameResultString.find_first_of(']', i + tagName.size() + 1) - 1;
            out = gameResultString.substr(left, right - left + 1);
            cur = right + 1;
            break;
        }
    }
}

int toNumericSQ(char colAlphabet, char rowNumber) {
    assert('A' <= colAlphabet && colAlphabet <= 'H');
    assert('1' <= rowNumber && rowNumber <= '8');
    return (7 - (colAlphabet - 'A')) + (7 - (rowNumber - '1')) * 8;
}

void convertGGFToRecordsAndWrite(const std::string& gameResultString, std::ofstream (&ofss)[60]) {
    int cur = 0;
    std::string boardString;
    getContentsOfNext("BO", gameResultString, cur, boardString);
    // 一般的な初期盤面以外は省く。
    if (boardString != "8 -------- -------- -------- ---O*--- ---*O--- -------- -------- -------- *") return;

    const std::string tags[] = { "B", "W" };
    Reversi reversi;
    std::vector<Reversi> pastPos;
    while (!reversi.isFinished) {
        std::string moveString;
        getContentsOfNext(tags[reversi.c], gameResultString, cur, moveString);
        int sq = toNumericSQ(moveString[0], moveString[1]);
        bool sucess = reversi.move(sq);

        if (!sucess) {
            std::cout << "an illegal move was detected" << std::endl;
            return;
        }

        pastPos.push_back(reversi);
    }

    for (int i = 0; i < pastPos.size(); ++i) {
        Recode record(
            pastPos[i].p,
            pastPos[i].o,
            pastPos[i].c == reversi.c
                ? popcount(reversi.p) - popcount(reversi.o)
                : popcount(reversi.o) - popcount(reversi.p));

        ofss[i].write((char*)&record, sizeof(Recode));
    }
}

void convertGGFDatabaseToRecords(const std::vector<std::string> &ggfDatabasePaths, std::string outputFolderPath) {
    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(addFileNameAtEnd(outputFolderPath, std::to_string(i + 1), "bin"), std::ios::binary);
    }

    for (std::string path : ggfDatabasePaths) {
        std::ifstream ifs(path);
        std::string gameResultString;
        std::getline(ifs, gameResultString);

        convertGGFToRecordsAndWrite(gameResultString, ofss);
    }
}