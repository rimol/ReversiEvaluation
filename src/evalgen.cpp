#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <time.h>
#include "bitboard.h"
#include "recode.h"
#include "eval.h"
#include "evalgen.h"

// 更新分の保存用
static int horDiff[8][6561];
static int verDiff[8][6561];
static int corDiff[4][6561];
static int mobDiff = 0;

// 結果はhorizontal, vertical, ...の配列に書き込む
// 評価値は最終石差*1000の近似とする
static void calculateEvaluationValue(std::string recodeFilePath) {
    // 更新の繰り返し回数
    constexpr int N = 100;
    // 配列の初期化（0埋め）
    clearArrays();
    // diffの方は0毎回0になってるはずですが一応
    std::fill((int*)horDiff, (int*)(horDiff + 8), 0);
    std::fill((int*)verDiff, (int*)(verDiff + 8), 0);
    std::fill((int*)corDiff, (int*)(corDiff + 4), 0);    
    mobDiff = 0;
    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::ifstream ifs(recodeFilePath, std::ios::ate | std::ios::binary);
    // ファイルに入っている局面の数
    const int M = ifs.tellg() / sizeof(Recode);
    std::vector<Recode> recodes(M);
    //beginから0バイトのところにストリーム位置を変更
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*)&recodes[0], M * sizeof(Recode));
    // N回ループ
    for (int k = 0; k < N; ++k) {
        for (Recode recode : recodes) {
            // 残差
            int r = (recode.result - evaluate(recode.p, recode.o)) * 1000;
            // このデータで出現する各特徴に対し更新分を加算していく
            // horver
            for (int i = 0; i < 8; ++i) {
                // hor[i], ver[i]
                Feature hor_i = extractHorizontal(i, recode.p, recode.o);
                Feature ver_i = extractVertical(i, recode.p, recode.o);
                horDiff[i][hor_i] += r;
                verDiff[i][ver_i] += r;
            }

            // corner
            for (int i = 0; i < 4; ++i) {
                Feature cor_i = extractCorner(i, recode.p, recode.o);
                corDiff[i][cor_i] += r;
            }

            // mobility
            mobDiff += r * getMobility(recode.p, recode.o);
        }

        // 更新分を適用。
        const int alpha = 57;
        // horver
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 6561; ++j) {
                horizontal[i][j] += horDiff[i][j] * alpha / N;
                vertical[i][j] += verDiff[i][j] * alpha / N;
                // 掃除もついでにする.
                horDiff[i][j] = verDiff[i][j] = 0;
            }
        }
        // corner
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 6561; ++j) {
                corner[i][j] += corDiff[i][j] * alpha / N;
                corDiff[i][j] = 0;
            }
        }
        // mobility
        mobility += mobDiff * alpha / N;
        mobDiff = 0;
    }
}

void generateEvaluationFiles(std::string recodesFolderPath, std::string outputFolderPath) {
    // 正しいフォルダ以外が指定されたときのことはめんどくさいので考えません
    std::stringstream ss0;
    std::stringstream ss1;
    ss0 << recodesFolderPath;
    ss1 << outputFolderPath;
    if (recodesFolderPath.back() != '\\') ss0 << '\\';
    if (outputFolderPath.back() != '\\') ss1 << '\\';
    ss1 << time(NULL) << "\\";
    // フォルダ作成
    mkdir(ss1.str().c_str());

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 1; i <= 60; ++i) {
        std::stringstream _ss0;
        _ss0 << ss0.str() << i << ".bin";
        // ファイルパスを渡して計算させる
        calculateEvaluationValue(_ss0.str());
        // 保存～
        std::stringstream _ss1;
        _ss1 << ss1.str() << i << ".bin";
        std::ofstream ofs(_ss1.str(), std::ios::binary);
        ofs.write((char*)horizontal, sizeof(int) * 8 * 6561);
        ofs.write((char*)vertical, sizeof(int) * 8 * 6561);
        ofs.write((char*)corner, sizeof(int) * 4 * 6561);
        ofs.write((char*)&mobility, sizeof(int));
    }
}