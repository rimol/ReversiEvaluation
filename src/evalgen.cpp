#include "evalgen.h"
#include "bitboard.h"
#include "eval.h"
#include "record.h"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <vector>

#define FOREACH_FEATURE_VALUE(Statement)            \
    for (int i = 0; i < GroupNum; ++i) {            \
        for (int j = 0; j < EvalArrayLength; ++j) { \
            auto &fv = featureValues[i][j];         \
            Statement                               \
        }                                           \
    }

#define FOREACH_FEATURE_IN(_recordEx, Statement)                              \
    for (int i = 0; i < FeatureNum; ++i) {                                    \
        int _g = Feature.group[i];                                            \
        Bitboard _p = (_recordEx).playerRotatedBB[Feature.rotationType[i]];   \
        Bitboard _o = (_recordEx).opponentRotatedBB[Feature.rotationType[i]]; \
        auto &fv = featureValues[_g][extract(_p, _o, _g)];                    \
        Statement                                                             \
    }

// 各特徴の評価値、評価値の更新分、ステップサイズをまとめて持つ
struct FeatureValue {
    double evalValue = 0.0;
    double evalValueUpdate = 0.0;
    double stepSize = 0.0;
};

static FeatureValue featureValues[GroupNum][EvalArrayLength];
static FeatureValue mobilityValue;
/*
    盤面を縦横4分割して、その分割した盤面の1つに対して、空きマスの数が偶数なら-1、奇数なら1となる特徴.
    偶数理論に対応するやつ. 終盤用の特徴になるはず. 序盤の精度が下がりそうで怖い（変に最適化されそうなので）
*/
static FeatureValue parityValue;

static inline void fillAllArraysAndVarialblesWithZero() {
    std::fill((FeatureValue *)featureValues, (FeatureValue *)(featureValues + GroupNum), FeatureValue());
    mobilityValue = FeatureValue();
    parityValue = FeatureValue();
}

// y - e
static inline double evalLoss(const RecordEx &recordEx) {
    double e = 0.0;
    FOREACH_FEATURE_IN(recordEx, { e += fv.evalValue; })
    e += (double)mobilityDiff(recordEx.playerRotatedBB[0], recordEx.opponentRotatedBB[0]) * mobilityValue.evalValue;
    e += (double)paritySum(recordEx.playerRotatedBB[0] | recordEx.opponentRotatedBB[0]) * parityValue.evalValue;
    return (double)recordEx.result - e;
}

static inline void applyUpdatesOfEvalValues() {
    FOREACH_FEATURE_VALUE(
        fv.evalValue += fv.evalValueUpdate * fv.stepSize;
        fv.evalValueUpdate = 0.0;)

    mobilityValue.evalValue += mobilityValue.evalValueUpdate * mobilityValue.stepSize;
    parityValue.evalValue += parityValue.evalValueUpdate * parityValue.stepSize;

    mobilityValue.evalValueUpdate = parityValue.evalValueUpdate = 0.0;
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(std::string recordFilePath, double beta) {
    fillAllArraysAndVarialblesWithZero();

    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::ifstream ifs(recordFilePath, std::ios::ate | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "Can't open the file: " << recordFilePath << std::endl;
        return -1.0;
    }

    // ファイルに入っている局面の数
    const int M = ifs.tellg() / sizeof(Record);
    std::vector<RecordEx> records(M);
    //beginから0バイトのところにストリーム位置を変更
    ifs.seekg(0, std::ios::beg);

    // 一つ一つ変換しながら読み込む
    for (int i = 0; i < M; ++i) {
        Record rec;
        ifs.read((char *)&rec, sizeof(Record));
        records[i] = RecordEx(rec);
    }

    // 予め各特徴のステップサイズを計算しておく。
    for (const auto &recordEx : records) {
        FOREACH_FEATURE_IN(recordEx, { ++fv.stepSize; })
    }

    FOREACH_FEATURE_VALUE(
        fv.stepSize = std::min(beta / 50.0, beta / fv.stepSize);)

    // これやるの忘れてたああああああああ
    mobilityValue.stepSize = parityValue.stepSize = beta / (double)M;

    double prevSquaredLossSum = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while (true) {
        // 偏差の2乗の和
        double squaredLossSum = 0;
        for (const auto &recordEx : records) {
            // 残差
            double loss = evalLoss(recordEx);
            squaredLossSum += loss * loss;
            // このデータで出現する各特徴に対し更新分を加算していく
            FOREACH_FEATURE_IN(recordEx, { fv.evalValueUpdate += loss; })

            mobilityValue.evalValueUpdate += loss * (double)mobilityDiff(recordEx.playerRotatedBB[0], recordEx.opponentRotatedBB[0]);
            parityValue.evalValueUpdate += loss * (double)paritySum(recordEx.playerRotatedBB[0] | recordEx.opponentRotatedBB[0]);
        }

        double currentVariance = squaredLossSum / (double)M;

        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        // -> １局面あたりの変化（の二乗）がXを下回ったら終了する
        constexpr double X = 10e-8;
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

void generateEvaluationFiles(std::string recordsFolderPath, std::string outputFolderPath, double beta) {
    outputFolderPath = createCurrentTimeFolderIn(outputFolderPath);
    // 分散を保存するファイルを作る
    std::ofstream vofs(addFileNameAtEnd(outputFolderPath, "variance", "txt"));

    // (1-60).binについてそれぞれ計算→保存
    for (int i = 60; i >= 1; --i) {
        // ファイルパスを渡して計算させる
        double variance = calculateEvaluationValue(addFileNameAtEnd(recordsFolderPath, std::to_string(i), "bin"), beta);
        // 保存～
        vofs << i << ".bin: " << variance << std::endl;

        std::ofstream ofs(addFileNameAtEnd(outputFolderPath, std::to_string(i), "bin"), std::ios::binary);
        // 評価値のみ保存したいので仕方なしループ
        FOREACH_FEATURE_VALUE(
            ofs.write((char *)&fv.evalValue, sizeof(double));)
        ofs.write((char *)&mobilityValue.evalValue, sizeof(double));
        ofs.write((char *)&parityValue.evalValue, sizeof(double));
    }
}