//Thorデータベースの書式について: http://hp.vector.co.jp/authors/VA015468/platina/algo/append_a.html
//棋譜 http://www.ffothello.org/informatique/la-base-wthor/
// https://www.skatgame.net/mburo/ggs/game-archive/Othello/
#include <cstdint>
#include <string>
#include <vector>

// 終盤は完璧な手を打っている棋譜であると仮定しときます。
struct Thor {
    // ゴミ
    int16_t _, __, ___;
    int8_t ____;
    // 神
    int8_t _____;
    int8_t moves[60];
};

void convertDatabaseToRecord(const std::string &folderpath, const std::string &outputFolderpath);
void mergeRecordFiles(std::string folderPath0, std::string folderPath1);