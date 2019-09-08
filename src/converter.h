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
// 完全読みによる訂正に加えて、途中で終局してる残念棋譜の削除、正しい手を打ってない棋譜の削除、重複した棋譜（回転して同じものも含む）の削除を行う
void correctRecords(const std::string &recordFolderpath, const std::string &saveFolderpath, int exactDepth, Solver &solver);
// outputのフォルダ内のi.binはまずクリアされるので注意
void mergeTrainingDataFiles(const std::vector<std::string> &inputFolderpaths, const std::string &outputFolderpath);
void compressEvalFiles(const std::string &weightFolderpath, const std::string &saveFolderpath);