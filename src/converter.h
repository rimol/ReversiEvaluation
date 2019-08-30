//Thorデータベースの書式について: http://hp.vector.co.jp/authors/VA015468/platina/algo/append_a.html
//棋譜 http://www.ffothello.org/informatique/la-base-wthor/
// https://www.skatgame.net/mburo/ggs/game-archive/Othello/
#include "solver.h"
#include <cstdint>
#include <string>
#include <vector>

// 拡張子が.ggfならGGF形式の、.wtbならThor形式で読み込んで変換する（ちゃんと拡張子をつけてないと読み込めない）。
void convertDatabaseToRecords(const std::string &folderpath, const std::string &outputFolderpath);
// 教師データのフォーマットに変換 saveFolderpathに1.bin...みたいなのが生成される
void generateTrainingData(const std::string &recordFolderpath, const std::string &saveFolderpath);
// 棋譜訂正
void correctRecords(const std::string &recordFolderpath, const std::string &saveFolderpath, int exactDepth, Solver &solver);
// outputのフォルダ内のi.binはまずクリアされるので注意
void mergeTrainingDataFiles(const std::vector<std::string> &inputFolderpaths, const std::string &outputFolderpath);
// record.pに常に黒石ビット、oに常に白石ビットが入るように局面データを修正する関数
// 今更だけど、Recordじゃなくて、Positionがクラス名として適当では？ｗ
// void fixRecords(const std::string &inputFolderpath, const std::string &outputFolderpath);