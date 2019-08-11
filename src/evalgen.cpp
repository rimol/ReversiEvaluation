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

constexpr double beta = 0.01;

// 各特徴の評価値、評価値の更新分、ステップサイズをまとめて持つ
struct FeatureValue {
    double weight = 0.0;
    double update = 0.0;
    double stepSize = 0.0;
};

static FeatureValue featureValues[GroupNum][EvalArrayLength];
static FeatureValue mobilityValue;
/*
    盤面を縦横4分割して、その分割した盤面の1つに対して、空きマスの数が偶数なら-1、奇数なら1となる特徴.
    偶数理論に対応するやつ. 終盤用の特徴になるはず. 序盤の精度が下がりそうで怖い（変に最適化されそうなので）
*/
static FeatureValue parityValue;
static FeatureValue stoneDiffValue;

static inline void fillAllArraysAndVarialblesWithZero() {
    std::fill((FeatureValue *)featureValues, (FeatureValue *)(featureValues + GroupNum), FeatureValue());
    mobilityValue = FeatureValue();
    parityValue = FeatureValue();
    stoneDiffValue = FeatureValue();
}

// y - e
static inline double evalLoss(const RecordEx &recordEx) {
    double e = 0.0;
    FOREACH_FEATURE_IN(recordEx, { e += fv.weight; })
    e += (double)mobilityDiff(recordEx.playerRotatedBB[0], recordEx.opponentRotatedBB[0]) * mobilityValue.weight;
    e += (double)paritySum(recordEx.playerRotatedBB[0] | recordEx.opponentRotatedBB[0]) * parityValue.weight;
    e += (double)(popcount(recordEx.playerRotatedBB[0]) - popcount(recordEx.opponentRotatedBB[0])) * stoneDiffValue.weight;
    return (double)recordEx.result - e;
}

static inline void applyUpdatesOfEvalValues() {
    FOREACH_FEATURE_VALUE(
        fv.weight += fv.update * fv.stepSize;
        fv.update = 0.0;)

    mobilityValue.weight += mobilityValue.update * mobilityValue.stepSize;
    parityValue.weight += parityValue.update * parityValue.stepSize;
    stoneDiffValue.weight += stoneDiffValue.update * stoneDiffValue.stepSize;

    mobilityValue.update = parityValue.update = stoneDiffValue.update = 0.0;
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
static double calculateEvaluationValue(const std::vector<std::string> &recordFilepaths) {
    fillAllArraysAndVarialblesWithZero();

    int numUsedRecords = 0;

    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::vector<RecordEx> records;
    for (auto &filepath : recordFilepaths) {
        std::ifstream ifs(filepath, std::ios::ate | std::ios::binary);
        if (!ifs.is_open()) {
            std::cout << "Can't open a file: " << filepath << std::endl;
        } else {
            int numRecordInThisFile = ifs.tellg() / sizeof(Record);
            numUsedRecords += numRecordInThisFile;
            ifs.seekg(0);

            for (int i = 0; i < numRecordInThisFile; ++i) {
                Record record;
                ifs.read((char *)&record, sizeof(Record));
                assert((record.p & record.o) == 0ULL);
                records.emplace_back(record);
            }
        }
    }

    // 予め各特徴のステップサイズを計算しておく。
    for (const auto &recordEx : records) {
        FOREACH_FEATURE_IN(recordEx, { ++fv.stepSize; })
    }

    FOREACH_FEATURE_VALUE(
        fv.stepSize = std::min(beta / 50.0, beta / fv.stepSize);)

    // これやるの忘れてたああああああああ
    mobilityValue.stepSize = parityValue.stepSize = beta / (double)numUsedRecords;
    stoneDiffValue.stepSize = beta / (double)numUsedRecords / 4.0;

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
            FOREACH_FEATURE_IN(recordEx, { fv.update += loss; })

            mobilityValue.update += loss * (double)mobilityDiff(recordEx.playerRotatedBB[0], recordEx.opponentRotatedBB[0]);
            parityValue.update += loss * (double)paritySum(recordEx.playerRotatedBB[0] | recordEx.opponentRotatedBB[0]);
            stoneDiffValue.update += loss * (double)(popcount(recordEx.playerRotatedBB[0]) - popcount(recordEx.opponentRotatedBB[0]));
        }

        double currentVariance = squaredLossSum / (double)numUsedRecords;

        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        // -> １局面あたりの変化（の二乗）がXを下回ったら終了する
        constexpr double X = 10e-4;
        if (std::abs(squaredLossSum - prevSquaredLossSum) / (double)numUsedRecords < X) {
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

void generateEvaluationFiles(std::string recordsFolderPath, std::string outputFolderPath, int first, int last) {
    if (first <= last && 1 <= first && last <= NumStages) {
        outputFolderPath = createCurrentTimeFolderIn(outputFolderPath);
        // 分散を保存するファイルを作る
        std::ofstream vofs(addFileNameAtEnd(outputFolderPath, "variance", "txt"));

        for (int i = first; i <= last; ++i) {
            std::vector<std::string> folderpaths;
            for (int j = 0; j < StageInterval; ++j) {
                folderpaths.push_back(addFileNameAtEnd(recordsFolderPath, std::to_string(i * 4 - j), "bin"));
            }

            // ファイルパスを渡して計算させる
            double variance = calculateEvaluationValue(folderpaths);
            // 保存～
            vofs << i << ".bin: " << variance << std::endl;

            std::ofstream ofs(addFileNameAtEnd(outputFolderPath, std::to_string(i), "bin"), std::ios::binary);
            // 評価値のみ保存したいので仕方なしループ
            FOREACH_FEATURE_VALUE(
                ofs.write((char *)&fv.weight, sizeof(double));)
            ofs.write((char *)&mobilityValue.weight, sizeof(double));
            ofs.write((char *)&parityValue.weight, sizeof(double));
            ofs.write((char *)&stoneDiffValue.weight, sizeof(double));
        }
    }
}