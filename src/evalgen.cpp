#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include "bitboard.h"
#include "recode.h"
#include "eval.h"
#include "evalgen.h"
#include "util.h"

static double evalValues[GroupNum][EvalArrayLength];
static double mobilityWeight;
static double intercept;

// 更新分の保存用
static double evalValueUpdates[GroupNum][EvalArrayLength];
static double mobilityUpdate;
static double interceptUpdate;

// 使う棋譜中で各特徴が出現する回数
// ステップサイズを決めるのに使う
static int featureFrequency[GroupNum][EvalArrayLength];

// y - e
static double evalLoss(const RecodeEx& recodeEx) {
    int t = popcount(recodeEx.playerRotatedBB[0] | recodeEx.opponentRotatedBB[0]) - 4 - 1;
    double e = intercept;
    for (int i = 0; i < FeatureNum; ++i) {
        int g = Feature.group[i];
        Bitboard p_ = recodeEx.playerRotatedBB[Feature.rotationType[i]];
        Bitboard o_ = recodeEx.opponentRotatedBB[Feature.rotationType[i]];

        e += evalValues[g][extract(p_, o_, g)];
    }
    
    e += (double)getMobility(recodeEx.playerRotatedBB[0], recodeEx.opponentRotatedBB[0]) * mobilityWeight;
    return (double)recodeEx.result - e;
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(std::string recodeFilePath, double beta) {
    // 配列の初期化（0埋め）
    std::fill((double*)evalValues, (double*)(evalValues + GroupNum), 0);
    std::fill((int*)featureFrequency, (int*)(featureFrequency + GroupNum), 0);
    mobilityWeight = mobilityUpdate = intercept = interceptUpdate = 0.0;

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
            Bitboard p_ = recodeEx.playerRotatedBB[Feature.rotationType[i]];
            Bitboard o_ = recodeEx.opponentRotatedBB[Feature.rotationType[i]];
            ++featureFrequency[g][extract(p_, o_, g)];
        }
    }

    double prevSquaredLossSum = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while(true) {
        // 偏差の2乗の和
        double squaredLossSum = 0;
        for (RecodeEx recodeEx : recodes) {
            // 残差
            double loss = evalLoss(recodeEx);
            squaredLossSum += loss * loss;
            // このデータで出現する各特徴に対し更新分を加算していく
            for (int i = 0; i < FeatureNum; ++i) {
                int g = Feature.group[i];
                Bitboard p_ = recodeEx.playerRotatedBB[Feature.rotationType[i]];
                Bitboard o_ = recodeEx.opponentRotatedBB[Feature.rotationType[i]];
                evalValueUpdates[g][extract(p_, o_, g)] += loss;
            }

            // mobility
            mobilityUpdate += loss * getMobility(recodeEx.playerRotatedBB[0], recodeEx.opponentRotatedBB[0]);
            interceptUpdate += loss;
        }

        double currentVariance = squaredLossSum / (double)M;

        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        if (std::abs(squaredLossSum - prevSquaredLossSum) < 10e-6) {
            std::cout << "Done. variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
            return currentVariance;
        }

        prevSquaredLossSum = squaredLossSum;

        // update
        for (int i = 0; i < GroupNum; ++i) {
            for (int j = 0; j < EvalArrayLength; ++j) {
                evalValues[i][j] += evalValueUpdates[i][j] * std::min(beta / 50.0, beta / (double)featureFrequency[i][j]);
                evalValueUpdates[i][j] = 0.0;
            }
        }
        mobilityWeight += mobilityUpdate * beta / (double)M;
        intercept += interceptUpdate * beta / (double)M;

        mobilityUpdate = interceptUpdate = 0.0;

        if (++loopCounter % 1000 == 0) {
            std::cout << "current variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
        }
    }
}

void generateEvaluationFiles(std::string recodesFolderPath, std::string outputFolderPath, double beta) {
    outputFolderPath = createCurrentTimeFolderIn(recodesFolderPath);
    // 分散を保存するファイルを作る
    std::ofstream vofs(addFileNameAtEnd(outputFolderPath, "variance", "txt"));

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 60; i >= 1; --i) {
        std::string saveFilePath = addFileNameAtEnd(outputFolderPath, std::to_string(i), "bin");
        // ファイルパスを渡して計算させる
        double variance = calculateEvaluationValue(saveFilePath, beta);
        // 保存～
        vofs << i << ".bin: " << variance << std::endl;

        std::ofstream ofs(saveFilePath, std::ios::binary);
        ofs.write((char*)evalValues, sizeof(double) * GroupNum * EvalArrayLength);
        ofs.write((char*)&mobilityWeight, sizeof(double));
        ofs.write((char*)&intercept, sizeof(double));
    }
}