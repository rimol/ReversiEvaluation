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

#define FOREACH_FEATURE_VALUE(Statement) \
for (int i = 0; i < GroupNum; ++i) { \
    for (int j = 0; j < EvalArrayLength; ++j) { \
        auto& fv = featureValues[i][j]; \
        Statement \
    } \
} \

#define FOREACH_FEATURE_IN(_recodeEx, Statement) \
for (int i = 0; i < FeatureNum; ++i) { \
    int _g = Feature.group[i]; \
    Bitboard _p = (_recodeEx).playerRotatedBB[Feature.rotationType[i]]; \
    Bitboard _o = (_recodeEx).opponentRotatedBB[Feature.rotationType[i]]; \
    auto& fv = featureValues[_g][extract(_p, _o, _g)]; \
    Statement \
} \

// 各特徴の評価値、評価値の更新分、ステップサイズをまとめて持つ
struct FeatureValue {
    double evalValue = 0.0;
    double evalValueUpdate = 0.0;
    double stepSize = 0.0;
};

static FeatureValue featureValues[GroupNum][EvalArrayLength];

static double mobilityWeight;
static double intercept;

// 更新分の保存用
static double mobilityUpdate;
static double interceptUpdate;

// stepSizes[i][j] = min(beta/50, beta/(特徴ijが出現する回数));
static double stepSize2;

static void fillAllArraysAndVarialblesWithZero() {
    std::fill((FeatureValue*)featureValues, (FeatureValue*)(featureValues + GroupNum), FeatureValue());
    mobilityWeight = mobilityUpdate = intercept = interceptUpdate = stepSize2 = 0.0;
}

// y - e
static double evalLoss(const RecodeEx& recodeEx) {
    double e = intercept;

    FOREACH_FEATURE_IN(recodeEx, { e += fv.evalValue; })

    e += (double)getMobility(recodeEx.playerRotatedBB[0], recodeEx.opponentRotatedBB[0]) * mobilityWeight;
    return (double)recodeEx.result - e;
}

static void applyUpdatesOfEvalValues() {
    FOREACH_FEATURE_VALUE(
        fv.evalValue += fv.evalValueUpdate * fv.stepSize;
        fv.evalValueUpdate = 0.0;
    )

    mobilityWeight += mobilityUpdate * stepSize2;
    intercept += interceptUpdate * stepSize2;

    mobilityUpdate = interceptUpdate = 0.0;
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(std::string recodeFilePath, double beta) {
    fillAllArraysAndVarialblesWithZero();

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

    // 予め各特徴のステップサイズを計算しておく。
    for (const auto& recodeEx : recodes) {
        FOREACH_FEATURE_IN(recodeEx, { ++fv.stepSize; })
    }

    FOREACH_FEATURE_VALUE(
        fv.stepSize = std::min(beta / 50.0, beta / fv.stepSize);
    )

    double prevSquaredLossSum = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while(true) {
        // 偏差の2乗の和
        double squaredLossSum = 0;
        for (const auto& recodeEx : recodes) {
            // 残差
            double loss = evalLoss(recodeEx);
            squaredLossSum += loss * loss;
            // このデータで出現する各特徴に対し更新分を加算していく
            FOREACH_FEATURE_IN(recodeEx, { fv.evalValueUpdate += loss; })

            // mobility
            mobilityUpdate += loss * getMobility(recodeEx.playerRotatedBB[0], recodeEx.opponentRotatedBB[0]);
            interceptUpdate += loss;
        }

        double currentVariance = squaredLossSum / (double)M;

        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        // -> １局面あたりの変化（の二乗）がXを下回ったら終了する
        constexpr double X = 10e-4;
        if (std::abs(squaredLossSum - prevSquaredLossSum) / (double)M < X) {
            std::cout << "Done. variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
            return currentVariance;
        }

        prevSquaredLossSum = squaredLossSum;

        applyUpdatesOfEvalValues();

        if (++loopCounter % 10 == 0) {
            std::cout << "current variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
        }
    }
}

void generateEvaluationFiles(std::string recodesFolderPath, std::string outputFolderPath, double beta) {
    outputFolderPath = createCurrentTimeFolderIn(outputFolderPath);
    // 分散を保存するファイルを作る
    std::ofstream vofs(addFileNameAtEnd(outputFolderPath, "variance", "txt"));

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 60; i >= 1; --i) {
        // ファイルパスを渡して計算させる
        double variance = calculateEvaluationValue(addFileNameAtEnd(recodesFolderPath, std::to_string(i), "bin"), beta);
        // 保存～
        vofs << i << ".bin: " << variance << std::endl;

        std::ofstream ofs(addFileNameAtEnd(outputFolderPath, std::to_string(i), "bin"), std::ios::binary);
        // 評価値のみ保存したいので仕方なしループ
        FOREACH_FEATURE_VALUE(
            ofs.write((char*)&fv.evalValue, sizeof(double));
        )
        ofs.write((char*)&mobilityWeight, sizeof(double));
        ofs.write((char*)&intercept, sizeof(double));
    }
}