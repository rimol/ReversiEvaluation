#include <cassert>
#include <cctype>
#include <fstream>
#include "record.h"
#include "reversi.h"
#include "converter.h"
#include "util.h"

void convertThorDatabaseToRecordFiles(const std::vector<std::string> &thorDatabasePaths, std::string outputFolderPath) {
    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(addFileNameAtEnd(outputFolderPath, std::to_string(i + 1), "bin"), std::ios::binary);
    }

    int totalRecordNum = 0;

    for (auto path : thorDatabasePaths) {
        std::ifstream ifs(path, std::ios::ate | std::ios::binary);
        if (!ifs) {
            std::cout << "entered path is invalid" << std::endl;
            continue;
        }
        // databaseに入っている試合数を割り出す
        const int M = ((int)ifs.tellg() - 16) / sizeof(Thor);
        std::vector<Thor> database(M);
        totalRecordNum += M;
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

                Record record(reversi.p, reversi.o, result);

                // 黒目線なので必要があればひっくり返す。
                if (reversi.c == White) {
                    record.result *= -1;
                }

                ofss[i].write((char*)&record, sizeof(Record));
            }
        }
    }

    std::cout << "Total: " << totalRecordNum << " records were converted!" << std::endl;
}

class StringWithCursor {
    int cur = 0;
    std::string str;

    public:

    StringWithCursor(std::string str) : str(str) {}
    char get(int i) const { return str[i]; }
    int size() const { return str.size(); }
    int cursor() const { return cur; }

    bool findFirstOf(char c, int first, int& out) const {
        out = str.find_first_of(c, first);
        return out != -1;
    }

    void substr(int first, int length, std::string& out) const {
        out = str.substr(first, length);
    }

    void setCursor(int i) { cur = i; }
};

/*
'tagName'["ここを抜き出す"]
抜き出せなかったらfalse
抜き出す内容が空の場合があるがそれはtrueにしとく。
抜き出せたらカーソルを動かす
 */
bool getNextContents(std::string tagName, StringWithCursor& swc, std::string& out) {
    for (int i = swc.cursor(); i < (swc.size() - tagName.size() - 2); ++i) {
        bool found = true;
        for (int j = 0; j < tagName.size(); ++j) {
            if (swc.get(i + j) != tagName[j]) {
                found = false;
                break;
            }
        }

        if (found && swc.get(i + tagName.size()) == '[') {
            // [の位置
            int first = i + tagName.size();
            // ]の位置
            int last;

            if (swc.findFirstOf(']', i + tagName.size() + 1, last)) {
                swc.substr(first + 1, last - first - 1, out);
                swc.setCursor(last + 1);
                return true;
            }
        }
    }

    return false;
}

int toNumericSQ(const std::string& moveString) {
    char colAlphabet = moveString[0];
    char rowNumber = moveString[1];

    char base = 'a' <= colAlphabet && colAlphabet <= 'h' ? 'a' : 'A';

    return (7 - (colAlphabet - base)) + (7 - (rowNumber - '1')) * 8;
}

bool isValidMoveString(const std::string& moveString) {
    if (moveString.size() < 2) return false;

    char colAlphabet = moveString[0];
    char rowNumber = moveString[1];

    return ('1' <= rowNumber && rowNumber <= '8')
        && (('a' <= colAlphabet && colAlphabet <= 'h') || ('A' <= colAlphabet && colAlphabet <= 'H'));
}

void convertGGFToRecordsAndWrite(const std::string& gameResultString, std::ofstream (&ofss)[60]) {
    StringWithCursor swc(gameResultString);
    std::string boardString;

    if (!getNextContents("BO", swc, boardString)) return;

    // 一般的な初期盤面以外は省く。
    if (boardString != "8 -------- -------- -------- ---O*--- ---*O--- -------- -------- -------- *") return;
    
    const std::string tags[] = { "B", "W" };
    Reversi reversi;
    std::vector<Reversi> pastPos;
    while (!reversi.isFinished) {
        std::string moveString;
        // ファイルぶっ壊れてる旨のメッセージ出した方がよいです。
        if (!getNextContents(tags[reversi.c], swc, moveString)) return;
        if (!isValidMoveString(moveString)) return;

        int sq = toNumericSQ(moveString);
        bool sucess = reversi.move(sq);

        if (!sucess) {
            std::cout << "an illegal move was detected." << std::endl;
            return;
        }

        pastPos.push_back(reversi);
    }

    for (int i = 0; i < pastPos.size(); ++i) {
        Record record(
            pastPos[i].p,
            pastPos[i].o,
            pastPos[i].c == reversi.c
                ? popcount(reversi.p) - popcount(reversi.o)
                : popcount(reversi.o) - popcount(reversi.p));

        ofss[i].write((char*)&record, sizeof(Record));
    }
}

// (;[ここを抜き出す];) 抜き出せなかったらfalseを返す
bool extractNextGameResult(std::ifstream& ifs, std::string& out) {
    std::stringstream ss;
    bool leftFound = false;
    bool prevIsLeftBracket = false;
    bool prevIsSemicolon = false;
    while (true) {
        char c = ifs.get();
        // みつかる前に終端
        if (ifs.eof()) return false;

        if (leftFound) {
            if (c == ';') prevIsSemicolon = true;
            else if (c == ')' && prevIsSemicolon) {
                out = ss.str();
                return true;
            } 
            else if (c != ')' && prevIsSemicolon) {
                ss << ';' << c;
            }
            else ss << c;
        }
        else {
            if (c == '(') prevIsLeftBracket = true;
            else if (c == ';' && prevIsLeftBracket) leftFound = true;
        }
    }
}

void convertGGFDatabaseToRecords(const std::vector<std::string> &ggfDatabasePaths, std::string outputFolderPath) {
    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(addFileNameAtEnd(outputFolderPath, std::to_string(i + 1), "bin"), std::ios::binary);
    }

    for (std::string path : ggfDatabasePaths) {
        std::ifstream ifs(path);

        if (!ifs.is_open()) continue;

        std::string gameResultString;
        while (extractNextGameResult(ifs, gameResultString)) {
            convertGGFToRecordsAndWrite(gameResultString, ofss);
        }
    }
}

void mergeRecordFiles(std::string folderPath0, std::string folderPath1) {
    for (int i = 1; i <= 60; ++i) {
        std::ofstream ofs(addFileNameAtEnd(folderPath0, std::to_string(i), "bin"), std::ios::binary | std::ios::app);
        std::ifstream ifs(addFileNameAtEnd(folderPath1, std::to_string(i), "bin"), std::ios::binary | std::ios::ate);

        if (!ifs.is_open()) continue;
        if (!ofs.is_open()) continue;

        const int RecordNum = ifs.tellg() / sizeof(Record);
        ifs.seekg(0);

        std::vector<Record> tmp(RecordNum);
        ifs.read((char*)&tmp[0], RecordNum * sizeof(Record));
        ofs.write((char*)&tmp[0], RecordNum * sizeof(Record));
    }
}