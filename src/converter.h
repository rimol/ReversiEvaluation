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

// 拡張子が.ggfならGGF形式の、.wtbならThor形式で読み込んで変換する（ちゃんと拡張子をつけてないと読み込めない）。
void convertDatabaseToRecord(const std::string &folderpath, const std::string &outputFolderpath);
// outputのフォルダ内のi.binはまずクリアされるので注意
void mergeRecordFiles(const std::vector<std::string> &inputFolderpaths, const std::string &outputFolderpath);
// record.pに常に黒石ビット、oに常に白石ビットが入るように局面データを修正する関数
// 今更だけど、Recordじゃなくて、Positionがクラス名として適当では？ｗ
void fixRecords(const std::string &inputFolderpath, const std::string &outputFolderpath);