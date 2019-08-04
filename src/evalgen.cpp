#include "evalgen.h"
#include "bitboard.h"
#include "eval.h"
#include "record.h"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <time.h>
#include <vector>

#define FOREACH_FEATURE_VALUE(Statement)            \
    for (int i = 0; i < GroupNum; ++i) {            \
        for (int j = 0; j < EvalArrayLength; ++j) { \
            auto &fv = featureValues[stage][i][j];  \
            Statement                               \
        }                                           \
    }

#define FOREACH_FEATURE_IN(_recordEx, Statement)                              \
    for (int i = 0; i < FeatureNum; ++i) {                                    \
        int _g = Feature.group[i];                                            \
        Bitboard _p = (_recordEx).playerRotatedBB[Feature.rotationType[i]];   \
        Bitboard _o = (_recordEx).opponentRotatedBB[Feature.rotationType[i]]; \
        auto &fv = featureValues[stage][_g][extract(_p, _o, _g)];             \
        Statement                                                             \
    }

// 各特徴の評価値、評価値の更新分、ステップサイズをまとめて持つ
struct FeatureValue {
    double weight = 0.0;
    double update = 0.0;
    double stepSize = 0.0;
};

static FeatureValue featureValues[60][GroupNum][EvalArrayLength];
static FeatureValue mobilityValue[60];
/*
    盤面を縦横4分割して、その分割した盤面の1つに対して、空きマスの数が偶数なら-1、奇数なら1となる特徴.
    偶数理論に対応するやつ. 終盤用の特徴になるはず. 序盤の精度が下がりそうで怖い（変に最適化されそうなので）
*/
static FeatureValue parityValue[60];

static inline void fillAllArraysAndVarialblesWithZero() {
    std::fill((FeatureValue *)featureValues, (FeatureValue *)(featureValues + 60), FeatureValue());
    std::fill((FeatureValue *)mobilityValue, (FeatureValue *)(mobilityValue + 60), FeatureValue());
    std::fill((FeatureValue *)parityValue, (FeatureValue *)(parityValue + 60), FeatureValue());
}

// y - e
static inline double evalLoss(const RecordEx &recordEx, const int stage) {
    double e = 0.0;
    FOREACH_FEATURE_IN(recordEx, { e += fv.weight; })
    e += (double)mobilityDiff(recordEx.playerRotatedBB[0], recordEx.opponentRotatedBB[0]) * mobilityValue[stage].weight;
    e += (double)paritySum(recordEx.playerRotatedBB[0] | recordEx.opponentRotatedBB[0]) * parityValue[stage].weight;
    return (double)recordEx.result - e;
}

static inline void applyUpdatesOfEvalValues(const int stage) {
    FOREACH_FEATURE_VALUE(
        fv.weight += fv.update * fv.stepSize;
        fv.update = 0.0;)

    mobilityValue[stage].weight += mobilityValue[stage].update * mobilityValue[stage].stepSize;
    parityValue[stage].weight += parityValue[stage].update * parityValue[stage].stepSize;

    mobilityValue[stage].update = parityValue[stage].update = 0.0;
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(std::string recordFilePath, double beta, int stage) {
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
    mobilityValue[stage].stepSize = parityValue[stage].stepSize = beta / (double)M;

    double prevSquaredLossSum = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while (true) {
        // 偏差の2乗の和
        double squaredLossSum = 0;
        for (const auto &recordEx : records) {
            // 残差
            double loss = evalLoss(recordEx, stage);
            squaredLossSum += loss * loss;
            // このデータで出現する各特徴に対し更新分を加算していく
            FOREACH_FEATURE_IN(recordEx, { fv.update += loss; })

            mobilityValue[stage].update += loss * (double)mobilityDiff(recordEx.playerRotatedBB[0], recordEx.opponentRotatedBB[0]);
            parityValue[stage].update += loss * (double)paritySum(recordEx.playerRotatedBB[0] | recordEx.opponentRotatedBB[0]);
        }

        double currentVariance = squaredLossSum / (double)M;

        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        // -> １局面あたりの変化（の二乗）がXを下回ったら終了する
        // 10e-4 から 10e-6にして生成すると逆に弱くなったｗ
        constexpr double X = 10e-4;
        if (std::abs(squaredLossSum - prevSquaredLossSum) / (double)M < X) {
            std::cout << stage << ": Done. variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
            return currentVariance;
        }

        prevSquaredLossSum = squaredLossSum;

        applyUpdatesOfEvalValues(stage);

        if (++loopCounter % 10 == 0) {
            std::cout << stage << ": current variance: " << currentVariance << ", loop: " << loopCounter << " times" << std::endl;
        }
    }
}

void generateEvaluationFiles(std::string recordsFolderPath, std::string outputFolderPath, double beta) {
    outputFolderPath = createCurrentTimeFolderIn(outputFolderPath);
    // 分散を保存するファイルを作る
    std::ofstream vofs(addFileNameAtEnd(outputFolderPath, "variance", "txt"));

    fillAllArraysAndVarialblesWithZero();

    std::vector<std::thread> threads;
    std::mutex mtx;
    // (1-60).binについてそれぞれ計算→保存
    for (int i = 60; i >= 1; --i) {
        threads.emplace_back([&mtx, &vofs, recordsFolderPath, beta, i, outputFolderPath]() {
            double variance = calculateEvaluationValue(addFileNameAtEnd(recordsFolderPath, std::to_string(i), "bin"), beta, i);

            mtx.lock();
            vofs << i << ".bin: " << variance << std::endl;
            mtx.unlock();

            std::ofstream ofs(addFileNameAtEnd(outputFolderPath, std::to_string(i), "bin"), std::ios::binary);
            // マクロの弊害がここに..
            const int stage = i;
            // 評価値のみ保存したいので仕方なしループ
            FOREACH_FEATURE_VALUE(
                ofs.write((char *)&fv.weight, sizeof(double));)
            ofs.write((char *)&mobilityValue[i].weight, sizeof(double));
            ofs.write((char *)&parityValue[i].weight, sizeof(double));
        });
        threads.back().join();
    }

    // for (auto &t : threads) {
    //     t.join();
    // }
}