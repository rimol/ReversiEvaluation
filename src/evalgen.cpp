#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <time.h>
#include "bitboard.h"
#include "recode.h"
#include "eval.h"
#include "evalgen.h"

static double evaluationValues[GroupNum][6561];
static double mobilityWeight;
static double intercept;

// 更新分の保存用
static double valDiff[GroupNum][6561];
static double mobDiff;
static double interceptDiff;

// 使う棋譜中で各特徴が出現する回数
// ステップサイズを決めるのに使う
static int featureFrequency[GroupNum][6561];

static double evaluateWithCurrentWeight(const RecodeEx& recodeEx) {
    int t = popcount(recodeEx.p_r[0] | recodeEx.o_r[0]) - 4 - 1;
    double e = intercept;
    for (int i = 0; i < FeatureNum; ++i) {
        int g = Feature.group[i];
        Bitboard p_ = recodeEx.p_r[Feature.rotationType[i]];
        Bitboard o_ = recodeEx.o_r[Feature.rotationType[i]];

        e += evaluationValues[g][extract(p_, o_, g)];
    }
    
    return e + (double)getMobility(recodeEx.p_r[0], recodeEx.o_r[0]) * mobilityWeight;
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(std::string recodeFilePath, double beta) {
    // 配列の初期化（0埋め）
    std::fill((double*)evaluationValues, (double*)(evaluationValues + GroupNum), 0);
    std::fill((double*)featureFrequency, (double*)(featureFrequency + GroupNum), 0);
    mobilityWeight = mobDiff = intercept = interceptDiff = 0.0;

    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::ifstream ifs(recodeFilePath, std::ios::ate | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "Can't open the file: " << recodeFilePath << std::endl;
        return -1.0;
    }

    // ファイルに入っている局面の数
    const int M = ifs.tellg() / sizeof(Recode);
    std::vector<RecodeEx> recodes(M);
    //beginから0バイトのところにストリーム位置を変更
    ifs.seekg(0, std::ios::beg);

    // 一つ一つ変換しながら読み込む
    for (int i = 0; i < M; ++i) {
        Recode rec;
        ifs.read((char*)&rec, sizeof(Recode));
        recodes[i] = RecodeEx(rec);
    }

    // 特徴の出現回数を予め計算しておく。
    for (RecodeEx recodeEx : recodes) {
        for (int i = 0; i < FeatureNum; ++i) {
            int g = Feature.group[i];
            Bitboard p_ = recodeEx.p_r[Feature.rotationType[i]];
            Bitboard o_ = recodeEx.o_r[Feature.rotationType[i]];
            ++featureFrequency[g][extract(p_, o_, g)];
        }
    }

    double previousVariance = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while(true) {
        // 偏差の2乗の和
        double squaredError = 0;
        for (RecodeEx recodeEx : recodes) {
            // 残差
            double r = ((double)recodeEx.result - evaluateWithCurrentWeight(recodeEx));
            squaredError += r * r;
            // このデータで出現する各特徴に対し更新分を加算していく
            for (int i = 0; i < FeatureNum; ++i) {
                int g = Feature.group[i];
                Bitboard p_ = recodeEx.p_r[Feature.rotationType[i]];
                Bitboard o_ = recodeEx.o_r[Feature.rotationType[i]];
                valDiff[g][extract(p_, o_, g)] += r;
            }

            // mobility
            mobDiff += r * getMobility(recodeEx.p_r[0], recodeEx.o_r[0]);
            interceptDiff += r;
        }

        double currentVariance = squaredError / (double)M;
        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        if (std::abs(previousVariance - currentVariance) < 10e-6) {
            std::cout << "Done. variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
            return currentVariance;
        }

        previousVariance = currentVariance;

        // update
        for (int i = 0; i < GroupNum; ++i) {
            for (int j = 0; j < 6561; ++j) {
                evaluationValues[i][j] += valDiff[i][j] * std::min(beta / 50.0, beta / (double)featureFrequency[i][j]);
                valDiff[i][j] = 0.0;
            }
        }
        mobilityWeight += mobDiff * beta / (double)M;
        intercept += interceptDiff * beta / (double)M;

        mobDiff = interceptDiff = 0.0;

        if (++loopCounter % 1000 == 0) {
            std::cout << "current variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
        }
    }
}

void generateEvaluationFiles(std::filesystem::path recodesFolderPath, std::filesystem::path outputFolderPath, double beta) {
    outputFolderPath /= std::to_string(time(NULL));
    // フォルダ作成
    std::filesystem::create_directory(outputFolderPath);

    // 分散を保存するファイルを作る
    std::ofstream vofs((outputFolderPath / "variance.txt").string());

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 60; i >= 1; --i) {
        // ファイルパスを渡して計算させる
        double variance = calculateEvaluationValue((recodesFolderPath / (std::to_string(i) + ".bin")).string(), beta);
        // 保存～
        vofs << i << ".bin: " << variance << std::endl;
        std::ofstream ofs((outputFolderPath / (std::to_string(i) + ".bin")).string(), std::ios::binary);
        ofs.write((char*)evaluationValues, sizeof(double) * FeatureNum * 6561);
        ofs.write((char*)&mobilityWeight, sizeof(double));
        ofs.write((char*)&intercept, sizeof(double));
    }
}