#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <time.h>
#include "bitboard.h"
#include "recode.h"
#include "eval.h"
#include "evalgen.h"

// 更新分の保存用
static double horDiff[8][6561];
static double verDiff[8][6561];
static double corDiff[4][6561];
static double mobDiff = 0.0;

// 結果はhorizontal, vertical, ...の配列に書き込む
static void calculateEvaluationValue(std::string recodeFilePath, double beta) {
    // 配列の初期化（0埋め）
    clearArrays();
    // diffの方は0毎回0になってるはずですが一応
    std::fill((double*)horDiff, (double*)(horDiff + 8), 0);
    std::fill((double*)verDiff, (double*)(verDiff + 8), 0);
    std::fill((double*)corDiff, (double*)(corDiff + 4), 0);    
    mobDiff = 0.0;
    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::ifstream ifs(recodeFilePath, std::ios::ate | std::ios::binary);
    // ファイルに入っている局面の数
    const int M = ifs.tellg() / sizeof(Recode);
    std::vector<Recode> recodes(M);
    //beginから0バイトのところにストリーム位置を変更
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*)&recodes[0], M * sizeof(Recode));

    constexpr double allowableVariance = 5.0;
    // ループカウンタ
    long long counter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while(true) {
        // 偏差の2乗の和
        double squaredDeviationSum = 0;
        for (Recode recode : recodes) {
            // 残差
            double r = ((double)recode.result - evaluate(recode.p, recode.o));
            squaredDeviationSum += pow(r, 2);
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

        // 終了チェック
        if (squaredDeviationSum / (double)M <= allowableVariance) break;

        // 更新分を適用。
        const double alpha = beta / (double)M;
        // horver
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 6561; ++j) {
                horizontal[i][j] += horDiff[i][j] * alpha;
                vertical[i][j] += verDiff[i][j] * alpha;
                // 掃除もついでにする.
                horDiff[i][j] = verDiff[i][j] = 0.0;
            }
        }
        // corner
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 6561; ++j) {
                corner[i][j] += corDiff[i][j] * alpha;
                corDiff[i][j] = 0.0;
            }
        }
        // mobility
        mobility += mobDiff * alpha;
        mobDiff = 0.0;

        ++counter;

        if (counter % 1000 == 0) {
            std::cout << "Current variance is " << squaredDeviationSum / (double)M << ".\n";
        }
    }
}

void generateEvaluationFiles(std::string recodesFolderPath, std::string outputFolderPath, double beta) {
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
    for (int i = 30; i <= 60; ++i) {
        std::stringstream _ss0;
        _ss0 << ss0.str() << i << ".bin";
        // ファイルパスを渡して計算させる
        calculateEvaluationValue(_ss0.str(), beta);
        // 保存～
        std::stringstream _ss1;
        _ss1 << ss1.str() << i << ".bin";
        std::ofstream ofs(_ss1.str(), std::ios::binary);
        ofs.write((char*)horizontal, sizeof(double) * 8 * 6561);
        ofs.write((char*)vertical, sizeof(double) * 8 * 6561);
        ofs.write((char*)corner, sizeof(double) * 4 * 6561);
        ofs.write((char*)&mobility, sizeof(double));
    }
}