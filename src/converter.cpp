#include "converter.h"
#include "reversi.h"
#include "solver.h"
#include "trainpos.h"
#include "util.h"
#include <cassert>
#include <cctype>
#include <fstream>
#include <regex>

std::string getFilenameNoExtension(const std::string &filepath) {
    auto begin = filepath.find_last_of(PathDivider) + 1;
    auto end = filepath.find_last_of('.');
    return filepath.substr(begin, end - begin);
}

struct Thor {
    // ゴミ
    int16_t _, __, ___;
    int8_t ____;
    // 神
    int8_t _____;
    int8_t moves[60];
};

// 盤面外に打とうとするまでを変換する（60手以内で終局した場合、残りに変な値が入っている）
// 返り値は変換した棋譜の数
// 正しい棋譜かどうかは判定しない
int convertThorToRecords(const std::string &filepath, const std::string &saveFolderpath) {
    std::ifstream ifs(filepath, std::ios::ate | std::ios::binary);
    std::string saveFilepath = addFileNameAtEnd(saveFolderpath, getFilenameNoExtension(filepath), "txt");
    std::ofstream ofs(saveFilepath);

    if (!ifs.is_open()) {
        std::cout << "Can't open a file:" << filepath << std::endl;
        return 0;
    }

    if (!ofs.is_open()) {
        std::cout << "Can't open a file:" << saveFilepath << std::endl;
        return 0;
    }

    // databaseに入っている試合数を割り出す
    const int M = ((int)ifs.tellg() - 16) / sizeof(Thor);
    std::vector<Thor> database(M);
    // ヘッダーの16バイトを飛ばす
    ifs.seekg(16);
    ifs.read((char *)&database[0], M * sizeof(Thor));

    for (auto thor : database) {
        std::vector<int> moves;
        for (int i = 0; i < 60; ++i) {
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

            if (0 <= x && x < 8 && 0 <= y && y < 8) {
                int sq = x + y * 8;
                moves.push_back(sq);
            } else
                break;
        }

        for (int sq : moves) {
            ofs << convertToLegibleSQ(sq);
        }
        ofs << std::endl;
    }

    return M;
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

int toNumericSQ(char colAlphabet, char rowNumber) {
    char base = 'a' <= colAlphabet && colAlphabet <= 'h' ? 'a' : 'A';
    return (7 - (colAlphabet - base)) + (7 - (rowNumber - '1')) * 8;
}

int toNumericSQ(const std::string &moveString) {
    char colAlphabet = moveString[0];
    char rowNumber = moveString[1];
    return toNumericSQ(colAlphabet, rowNumber);
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

    while (moves.size() < 60) {
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
int convertGGFToRecords(const std::string &filepath, const std::string &saveFolderpath) {
    std::ifstream ifs(filepath);
    std::string saveFilepath = addFileNameAtEnd(saveFolderpath, getFilenameNoExtension(filepath), "txt");
    std::ofstream ofs(saveFilepath);

    if (!ifs.is_open()) {
        std::cout << "Can't open a file:" << filepath << std::endl;
        return 0;
    }

    if (!ofs.is_open()) {
        std::cout << "Can't open a file:" << saveFilepath << std::endl;
        return 0;
    }

    int numConverted = 0;
    std::string gameResultString;
    std::vector<int> moves;
    while (extractNextGameResult(ifs, gameResultString)) {
        moves.clear();
        if (parseGGFGameResultString(gameResultString, moves)) {
            for (int sq : moves) {
                ofs << convertToLegibleSQ(sq);
            }
            ofs << std::endl;
            ++numConverted;
        }
    }

    return numConverted;
}

std::string getExtension(const std::string &filepath) {
    int begin = filepath.find_last_of('.');
    return filepath.substr(begin, filepath.size() - begin);
}

void convertDatabaseToRecords(const std::string &folderpath, const std::string &outputFolderpath) {
    std::vector<std::string> filepaths;
    enumerateFilesIn(folderpath, filepaths);

    int num = 0;

    for (auto &path : filepaths) {
        std::string extension = getExtension(path);

        int n;
        if (extension == ".wtb") {
            n = convertThorToRecords(path, outputFolderpath);
        } else if (extension == ".ggf") {
            n = convertGGFToRecords(path, outputFolderpath);
        } else
            continue;

        num += n;
        std::cout << "Done. " << n << " games in " << path << " were successfully converted." << std::endl;
    }

    std::cout << "Done! " << num << " games were converted!" << std::endl;
}

void generateTrainingData(const std::string &recordFolderpath, const std::string &saveFolderpath) {
    std::ofstream ofs[60];
    for (int i = 1; i <= 60; ++i) {
        ofs[i - 1] = std::ofstream(addFileNameAtEnd(saveFolderpath, std::to_string(i), "bin"), std::ios::binary);
    }

    std::vector<std::string> recordFilepaths;
    enumerateFilesIn(recordFolderpath, recordFilepaths);

    for (const auto &filepath : recordFilepaths) {
        if (getExtension(filepath) != ".txt")
            continue;

        int randomDepth = 0;
        auto cur = filepath.rfind("rand");

        if (cur != std::string::npos) {
            cur += 4;
            while (cur < filepath.size() && '0' <= filepath[cur] && filepath[cur] <= '9') {
                randomDepth *= 10;
                randomDepth += filepath[cur++] - '0';
            }
        }

        std::string recordStr;
        int numConverted = 0;
        std::ifstream ifs(filepath);
        while (getline(ifs, recordStr)) {
            Reversi reversi;
            std::vector<Reversi> pos;

            bool success = true;

            for (int i = 0; i < recordStr.size() - 1; i += 2) {
                if (reversi.move(toNumericSQ(recordStr[i], recordStr[i + 1]))) {
                    pos.push_back(reversi);
                } else {
                    std::cerr << "Can't convert: " << recordStr << std::endl;
                    success = false;
                    break;
                }
            }

            if (reversi.isFinished) {
                int result = popcount(pos.back().p) - popcount(pos.back().o);
                Color resultColor = pos.back().c;
                for (int i = std::max(0, randomDepth - 1); i < pos.size(); ++i) {
                    TrainingPosition trainingPos(pos[i].p, pos[i].o, result);
                    if (pos[i].c != resultColor)
                        trainingPos.result *= -1;
                    ofs[i].write((char *)&trainingPos, sizeof(TrainingPosition));
                }
            }

            numConverted += (int)success;
        }
    }
}

void correctRecords(const std::string &recordFolderpath, const std::string &saveFolderpath, int exactDepth, Solver &solver) {
    std::vector<std::string> recordFilepaths;
    enumerateFilesIn(recordFolderpath, recordFilepaths);

    int numCorrected = 0;
    for (const auto &filepath : recordFilepaths) {
        if (getExtension(filepath) != ".txt")
            continue;

        std::string recordStr;
        std::ifstream ifs(filepath);
        std::ofstream ofs(addFileNameAtEnd(saveFolderpath, getFilenameNoExtension(filepath) + "exact" + std::to_string(exactDepth), "txt"));
        while (getline(ifs, recordStr)) {
            Reversi reversi;
            bool success = true;
            std::stringstream ss;
            for (int i = 0; (reversi.stoneCount() + exactDepth) < 64 && i < recordStr.size() - 1; i += 2) {
                if (reversi.move(toNumericSQ(recordStr[i], recordStr[i + 1]))) {
                    ss << recordStr[i] << recordStr[i + 1];
                } else {
                    std::cerr << "Can't convert: " << recordStr << std::endl;
                    success = false;
                    break;
                }
            }

            if (success) {
                ofs << ss.str();
                auto solution = solver.solve(reversi.p, reversi.o);
                for (int sq : solution.bestMoves) {
                    ofs << convertToLegibleSQ(sq);
                }
                ofs << std::endl;
                ++numCorrected;
            }

            if (numCorrected % 100 == 0) {
                std::cout << numCorrected << "games were corrected." << std::endl;
            }
        }
    }
}

void mergeTrainingDataFiles(const std::vector<std::string> &inputFolderpaths, const std::string &outputFolderpath) {
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

            const int NumPositions = ifs.tellg() / sizeof(TrainingPosition);
            ifs.seekg(0);
            std::vector<TrainingPosition> tmp(NumPositions);
            ifs.read((char *)&tmp[0], NumPositions * sizeof(TrainingPosition));
            ofs.write((char *)&tmp[0], NumPositions * sizeof(TrainingPosition));
        }
    }
}

// void fixRecords(const std::string &inputFolderpath, const std::string &outputFolderpath) {
//     std::ifstream manyifs[60];
//     std::ofstream manyofs[60];
//     for (int i = 0; i < 60; ++i) {
//         std::string inputFilepath = addFileNameAtEnd(inputFolderpath, std::to_string(i + 1), "bin");
//         std::string outputFilepath = addFileNameAtEnd(outputFolderpath, std::to_string(i + 1), "bin");
//         manyifs[i] = std::ifstream(inputFilepath, std::ios::binary);
//         manyofs[i] = std::ofstream(outputFilepath, std::ios::binary);

//         if (!manyifs[i].is_open()) {
//             std::cout << "Can't open a file: " << inputFilepath << std::endl;
//             return;
//         }

//         if (!manyofs[i].is_open()) {
//             std::cout << "Can't open a file: " << outputFilepath << std::endl;
//             return;
//         }
//     }

//     while (!manyifs[0].eof()) {
//         Reversi reversi;
//         int i = 0;
//         while (!reversi.isFinished) {
//             Record record;
//             manyifs[i].read((char *)&record, sizeof(Record));

//             if (manyifs[i].eof()) {
//                 break;
//             }

//             Bitboard sqbit = (record.p | record.o) ^ (reversi.p | reversi.o);

//             assert(popcount(sqbit) == 1);

//             if (reversi.move(tzcnt(sqbit))) {
//                 assert(reversi.p == record.p && reversi.o == record.o);
//                 if (reversi.c == White) {
//                     std::swap(record.p, record.o);
//                     record.result *= -1;
//                 }

//                 manyofs[i].write((char *)&record, sizeof(Record));
//             } else
//                 assert(false);

//             ++i;
//         }
//     }
// }