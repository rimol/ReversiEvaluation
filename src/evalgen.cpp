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
static double valDiff[FeatureNum][6561];
static double mobDiff = 0.0;
static double interceptDiff = 0.0;

// 使う棋譜中で各特徴が出現する回数
// ステップサイズを決めるのに使う
static int featureFrequency[FeatureNum][6561];

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(std::string recodeFilePath, double beta) {
    // 配列の初期化（0埋め）
    std::fill((double*)evaluationValues, (double*)(evaluationValues + FeatureNum), 0);
    std::fill((double*)featureFrequency, (double*)(featureFrequency + FeatureNum), 0);
    mobilityWeight = mobDiff = intercept = interceptDiff = 0.0;

    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::ifstream ifs(recodeFilePath, std::ios::ate | std::ios::binary);
    // ファイルに入っている局面の数
    const int M = ifs.tellg() / sizeof(Recode);
    std::vector<Recode> recodes(M);
    //beginから0バイトのところにストリーム位置を変更
    ifs.seekg(0, std::ios::beg);
    ifs.read((char*)&recodes[0], M * sizeof(Recode));

    // 特徴の出現回数を予め計算しておく。
    for (Recode recode : recodes) {
        for (int i = 0; i < FeatureNum; ++i) {
            ++featureFrequency[i][extract(recode.p, recode.o, i)];
        }
    }

    double previousVariance = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while(true) {
        // 偏差の2乗の和
        double squaredDeviationSum = 0;
        for (Recode recode : recodes) {
            // 残差
            double r = ((double)recode.result - evaluate(recode.p, recode.o));
            squaredDeviationSum += r * r;
            // このデータで出現する各特徴に対し更新分を加算していく
            for (int i = 0; i < FeatureNum; ++i) {
                valDiff[i][extract(recode.p, recode.o, i)] += r;
            }

            // mobility
            mobDiff += r * getMobility(recode.p, recode.o);
            interceptDiff += r;
        }

        double currentVariance = squaredDeviationSum / (double)M;
        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        if (std::abs(previousVariance - currentVariance) < 10e-6) {
            std::cout << "Done. variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
            return currentVariance;
        }

        previousVariance = currentVariance;

        // update
        for (int i = 0; i < FeatureNum; ++i) {
            for (int j = 0; j < 6561; ++j) {
                evaluationValues[i][j] += valDiff[i][j] * std::min(beta / 50.0, beta / (double)featureFrequency[i][j]);
                valDiff[i][j] = 0.0;
            }
        }
        mobilityWeight += mobDiff * beta / (double)M;
        intercept += interceptDiff * beta / (double)M;

        mobDiff = interceptDiff = 0.0;

        if (++loopCounter % 1000 == 0) {
            std::cout << "current variance: " << currentVariance << ", loop: " << loopCounter << " times";
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

    // 分散を保存するファイルを作る
    std::ofstream vofs((ss1.str() + "variance.txt").c_str());

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 60; i >= 1; --i) {
        std::stringstream _ss0;
        _ss0 << ss0.str() << i << ".bin";
        // ファイルパスを渡して計算させる
        double variance = calculateEvaluationValue(_ss0.str(), beta);
        // 保存～
        vofs << i << ".bin: " << variance << std::endl; 
        std::stringstream _ss1;
        _ss1 << ss1.str() << i << ".bin";
        std::ofstream ofs(_ss1.str(), std::ios::binary);
        ofs.write((char*)evaluationValues, sizeof(double) * FeatureNum * 6561);
        ofs.write((char*)&mobilityWeight, sizeof(double));
        ofs.write((char*)&intercept, sizeof(double));
    }
}