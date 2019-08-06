#include "converter.h"
#include "record.h"
#include "reversi.h"
#include "solver.h"
#include "util.h"
#include <cassert>
#include <cctype>
#include <fstream>

// 棋譜訂正の深さ
constexpr int ExactDepth = 16;

// 失敗したらfalse
// 棋譜訂正と、movesがおかしくないかもチェックする。
// ofsはbinaryモードで開いている前提
bool writeRecordsToFile(const std::vector<int> &moves, std::ofstream (&ofs)[60]) {
    Reversi reversi;
    std::vector<Reversi> pos;
    for (int m : moves) {
        if (reversi.move(m)) {
            pos.push_back(reversi);
        } else {
            std::cout << "an illegal move was detected. this records will be omitted." << std::endl;
            return false;
        }
    }

    Color solverUser = reversi.c;
    Solution solution = solve(reversi.p, reversi.o);

    for (int m : solution.bestMoves) {
        if (reversi.move(m)) {
            pos.push_back(reversi);
        } else {
            std::cout << "an illegal move was detected. this records will be omitted." << std::endl;
            return false;
        }
    }

    for (int i = 0; i < pos.size(); ++i) {
        Record record(pos[i].p, pos[i].o, solution.bestScore);
        if (pos[i].c != solverUser)
            record.result *= -1;

        ofs[i].write((char *)&record, sizeof(Record));
    }

    return true;
}

// 返り値は変換した棋譜の数
int convertThorToRecords(const std::string &filepath, std::ofstream (&ofs)[60]) {
    std::ifstream ifs(filepath, std::ios::ate | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "'" << filepath << "'"
                  << "is invalid." << std::endl;
        return 0;
    }

    // databaseに入っている試合数を割り出す
    const int M = ((int)ifs.tellg() - 16) / sizeof(Thor);
    std::vector<Thor> database(M);
    // ヘッダーの16バイトを飛ばす
    ifs.seekg(16);
    ifs.read((char *)&database[0], M * sizeof(Thor));

    int error = 0;

    for (auto thor : database) {
        std::vector<int> moves;
        for (int i = 0; i < 60 - ExactDepth; ++i) {
            /*
                1手目は必ずF5らしいがどうでもいい。
                以下に従ってマス番号を変換する

                    A	B	C	D	E	F	G	H
                1	11	12	13	14	15	16	17	18
                2	21	22	23	24	25	26	27	28
                3	31	32	33	34	35	36	37	38
                4	41	42	43	44	45	46	47	48
                5	51	52	53	54	55	56	57	58
                6	61	62	63	64	65	66	67	68
                7	71	72	73	74	75	76	77	78
                8	81	82	83	84	85	86	87	88

             */
            int x = 8 - thor.moves[i] % 10;
            int y = 8 - thor.moves[i] / 10;
            int sq = x + y * 8;

            moves.push_back(sq);
        }

        bool success = writeRecordsToFile(moves, ofs);
        if (!success)
            ++error;
    }

    return M - error;
}

class StringWithCursor {
    int cur = 0;
    std::string str;

public:
    StringWithCursor(std::string str) : str(str) {}
    char get(int i) const { return str[i]; }
    int size() const { return str.size(); }
    int cursor() const { return cur; }

    bool findFirstOf(char c, int first, int &out) const {
        out = str.find_first_of(c, first);
        return out != -1;
    }

    void substr(int first, int length, std::string &out) const {
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
bool getNextContents(const std::vector<std::string> &tagNames, StringWithCursor &swc, std::string &out) {
    for (int i = swc.cursor(); i < (swc.size() - 2); ++i) {
        for (auto &tagName : tagNames) {
            if (i >= (swc.size() - tagName.size() - 2))
                continue;

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
    }

    return false;
}

int toNumericSQ(const std::string &moveString) {
    char colAlphabet = moveString[0];
    char rowNumber = moveString[1];

    char base = 'a' <= colAlphabet && colAlphabet <= 'h' ? 'a' : 'A';

    return (7 - (colAlphabet - base)) + (7 - (rowNumber - '1')) * 8;
}

bool isValidMoveString(const std::string &moveString) {
    if (moveString.size() < 2)
        return false;

    char colAlphabet = moveString[0];
    char rowNumber = moveString[1];

    return ('1' <= rowNumber && rowNumber <= '8') && (('a' <= colAlphabet && colAlphabet <= 'h') || ('A' <= colAlphabet && colAlphabet <= 'H'));
}

// 成功したらtrue
bool parseGGFGameResultString(const std::string &gameResultString, std::vector<int> &moves) {
    StringWithCursor swc(gameResultString);
    std::string boardString;

    if (!getNextContents({"BO"}, swc, boardString))
        return false;

    // 一般的な初期盤面以外は省く。
    if (boardString != "8 -------- -------- -------- ---O*--- ---*O--- -------- -------- -------- *")
        return false;

    while (moves.size() < 60 - ExactDepth) {
        std::string moveString;
        // ファイルぶっ壊れてる旨のメッセージ出した方がよいです。
        if (!getNextContents({"B", "W"}, swc, moveString))
            return false;
        if (!isValidMoveString(moveString))
            return false;

        int sq = toNumericSQ(moveString);
        moves.push_back(sq);
    }

    return true;
}

// (;[ここを抜き出す];) 抜き出せなかったらfalseを返す
bool extractNextGameResult(std::ifstream &ifs, std::string &out) {
    std::stringstream ss;
    bool leftFound = false;
    bool prevIsLeftBracket = false;
    bool prevIsSemicolon = false;
    while (true) {
        char c = ifs.get();
        // みつかる前に終端
        if (ifs.eof())
            return false;

        if (leftFound) {
            if (c == ';')
                prevIsSemicolon = true;
            else if (c == ')' && prevIsSemicolon) {
                out = ss.str();
                return true;
            } else if (c != ')' && prevIsSemicolon) {
                ss << ';' << c;
            } else
                ss << c;
        } else {
            if (c == '(')
                prevIsLeftBracket = true;
            else if (c == ';' && prevIsLeftBracket)
                leftFound = true;
        }
    }
}

// 返り値は変換した棋譜の数
int convertGGFToRecords(const std::string &filepath, std::ofstream (&ofs)[60]) {
    std::ifstream ifs(filepath);

    if (!ifs.is_open()) {
        std::cout << "'" << filepath << "'"
                  << "is invalid." << std::endl;
        return 0;
    }

    int num = 0;
    std::string gameResultString;
    std::vector<int> moves;
    while (extractNextGameResult(ifs, gameResultString)) {
        moves.clear();
        bool success = parseGGFGameResultString(gameResultString, moves);
        if (success) {
            writeRecordsToFile(moves, ofs);
            ++num;
        }
    }

    return num;
}

std::string getExtension(const std::string &filepath) {
    int begin = filepath.find_last_of('.');
    return filepath.substr(begin, filepath.size() - begin);
}

void convertDatabaseToRecord(const std::string &folderpath, const std::string &outputFolderpath) {
    std::ofstream ofs[60];
    for (int i = 1; i <= 60; ++i) {
        ofs[i - 1] = std::ofstream(addFileNameAtEnd(outputFolderpath, std::to_string(i), "bin"), std::ios::binary);
    }

    std::vector<std::string> filepaths;
    enumerateFilesIn(folderpath, filepaths);

    int num = 0;

    for (auto &path : filepaths) {
        std::string extension = getExtension(path);

        if (extension == ".wtb") {
            num += convertThorToRecords(path, ofs);
        } else if (extension == ".ggf") {
            num += convertGGFToRecords(path, ofs);
        }
    }

    std::cout << "Done! " << num << " games were converted!" << std::endl;
}

void mergeRecordFiles(const std::vector<std::string> &inputFolderpaths, const std::string &outputFolderpath) {
    for (int i = 1; i <= 60; ++i) {
        std::string outputFilepath = addFileNameAtEnd(outputFolderpath, std::to_string(i), "bin");
        std::ofstream ofs(outputFilepath, std::ios::binary);

        if (!ofs.is_open()) {
            std::cout << "cannot open a file '" << outputFilepath << "'" << std::endl;
            continue;
        }

        for (auto &inputFolderpath : inputFolderpaths) {
            std::string inputFilepath = addFileNameAtEnd(inputFolderpath, std::to_string(i), "bin");
            std::ifstream ifs(inputFilepath, std::ios::binary | std::ios::ate);

            if (!ifs.is_open()) {
                std::cout << "cannot open a file '" << inputFilepath << "'" << std::endl;
                continue;
            }

            const int RecordNum = ifs.tellg() / sizeof(Record);
            ifs.seekg(0);
            std::vector<Record> tmp(RecordNum);
            ifs.read((char *)&tmp[0], RecordNum * sizeof(Record));
            ofs.write((char *)&tmp[0], RecordNum * sizeof(Record));
        }
    }
}