#include "evalgen.h"
#include "bitboard.h"
#include "eval.h"
#include "trainpos.h"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#define FOREACH_FEATURE_VALUE(Statement)               \
    for (int i = 0; i < usedPattern.numGroup(); ++i) { \
        for (int j = 0; j < MaxNumPattern; ++j) {      \
            auto &fv = featureValues[i][j];            \
            Statement                                  \
        }                                              \
    }

#define FOREACH_FEATURE_IN(_trainingPosEx, Statement)                         \
    for (int i = 0; i < usedPattern.numPattern(); ++i) {                      \
        int _g = usedPattern.group(i);                                        \
        Bitboard _p = (_trainingPosEx).rotatedP[usedPattern.rotationType(i)]; \
        Bitboard _o = (_trainingPosEx).rotatedO[usedPattern.rotationType(i)]; \
        auto &fv = featureValues[_g][usedPattern.extract(_p, _o, _g)];        \
        Statement                                                             \
    }

EvalGen::EvalGen(int numStages, const std::string &patternName) : numStages(numStages), patternName(patternName), usedPattern(Patterns.at(patternName)) {
    assert(60 % numStages == 0);
    stageInterval = 60 / numStages;
    featureValues = new FeatureValue *[usedPattern.numGroup()];
    for (int i = 0; i < usedPattern.numGroup(); ++i) {
        featureValues[i] = new FeatureValue[MaxNumPattern];
    }
}

EvalGen::~EvalGen() {
    for (int i = 0; i < usedPattern.numGroup(); ++i) {
        delete[] featureValues[i];
    }
    delete featureValues;
}

constexpr double beta = 0.01;

void EvalGen::clearFeatureValues() {
    for (int i = 0; i < usedPattern.numGroup(); ++i) {
        for (int j = 0; j < MaxNumPattern; ++j) {
            featureValues[i][j] = FeatureValue();
        }
    }
}

// y - e
inline double EvalGen::evalLoss(const TrainingPositionEx &trainingPosEx) {
    double e = 0.0;
    FOREACH_FEATURE_IN(trainingPosEx, { e += fv.weight; })
    return (double)trainingPosEx.result - e;
}

inline void EvalGen::applyUpdatesOfEvalValues() {
    FOREACH_FEATURE_VALUE(
        fv.weight += fv.update * fv.stepSize;
        fv.update = 0.0;)
}

// 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
double EvalGen::calculateEvaluationValue(const std::vector<std::string> &trainingDataFilepaths) {
    clearFeatureValues();

    int numUsedPositions = 0;

    // ファイルを何回も読むのは無駄なので最初に全部読み込む
    std::vector<TrainingPositionEx> trainingPositions;
    for (auto &filepath : trainingDataFilepaths) {
        std::ifstream ifs(filepath, std::ios::ate | std::ios::binary);
        if (!ifs.is_open()) {
            std::cout << "Can't open a file: " << filepath << std::endl;
        } else {
            int numPositionsInThisFile = ifs.tellg() / sizeof(TrainingPosition);
            numUsedPositions += numPositionsInThisFile;
            ifs.seekg(0);

            for (int i = 0; i < numPositionsInThisFile; ++i) {
                TrainingPosition pos;
                ifs.read((char *)&pos, sizeof(TrainingPosition));
                assert((pos.p & pos.o) == 0ULL);
                trainingPositions.emplace_back(pos);
            }
        }
    }

    // 予め各特徴のステップサイズを計算しておく。
    for (const auto &trainingPosEx : trainingPositions) {
        FOREACH_FEATURE_IN(trainingPosEx, { ++fv.stepSize; })
    }

    FOREACH_FEATURE_VALUE(
        fv.stepSize = std::min(beta / 50.0, beta / fv.stepSize);)

    double prevSquaredLossSum = 0.0;
    long long loopCounter = 0;
    // ピピーーッ！無限ループ！！逮捕！！
    while (true) {
        // 偏差の2乗の和
        double squaredLossSum = 0;
        for (const auto &trainingPosEx : trainingPositions) {
            // 残差
            double loss = evalLoss(trainingPosEx);
            squaredLossSum += loss * loss; // Los!
            // このデータで出現する各特徴に対し更新分を加算していく
            FOREACH_FEATURE_IN(trainingPosEx, { fv.update += loss; })
        }

        double currentVariance = squaredLossSum / (double)numUsedPositions;

        // 終了条件わからん
        // 「前回との差がピッタリ0」を条件にすると終わらない
        // -> １局面あたりの変化（の二乗）がXを下回ったら終了する
        constexpr double X = 10e-4;
        if (std::abs(squaredLossSum - prevSquaredLossSum) / (double)numUsedPositions < X) {
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

void EvalGen::run(const std::string &trainingDataFolderPath, const std::string &outputFolderPath, int first, int last) {
    if (first <= last && 1 <= first && last <= numStages) {
        std::string saveFolderPath = createCurrentTimeFolderIn(outputFolderPath);
        // info.txt
        std::ofstream info_ofs(addFileNameAtEnd(saveFolderPath, "info", "txt"));
        info_ofs << patternName << " " << numStages;
        info_ofs.close();
        // 分散を保存するファイルを作る
        std::ofstream vofs(addFileNameAtEnd(saveFolderPath, "variance", "txt"));

        for (int i = first; i <= last; ++i) {
            std::vector<std::string> folderpaths;
            for (int j = 0; j < stageInterval; ++j) {
                folderpaths.push_back(addFileNameAtEnd(trainingDataFolderPath, std::to_string(i * 4 - j), "bin"));
            }

            // ファイルパスを渡して計算させる
            double variance = calculateEvaluationValue(folderpaths);
            // 保存～
            vofs << i << ".bin: " << variance << std::endl;

            std::ofstream ofs(addFileNameAtEnd(saveFolderPath, std::to_string(i), "bin"), std::ios::binary);
            // 評価値のみ保存したいので仕方なしループ
            FOREACH_FEATURE_VALUE(
                ofs.write((char *)&fv.weight, sizeof(double));)
        }
    }
}

// void printPatternCoverage(const std::string &recordsFolderPath) {
//     for (int i = 0; i < NumStages; ++i) {
//         std::set<int> occurrence[GroupNum];
//         for (int j = i * StageInterval; j < (i + 1) * StageInterval; ++j) {
//             std::ifstream ifs(addFileNameAtEnd(recordsFolderPath, std::to_string(j + 1), "bin"), std::ios::ate | std::ios::binary);
//             int filesize_byte = ifs.tellg();
//             ifs.seekg(0);
//             std::vector<Record> records(filesize_byte / sizeof(Record));
//             ifs.read((char *)&records[0], filesize_byte);

//             for (Record &record : records) {
//                 RecordEx recodeEx(record);
//                 for (int k = 0; k < FeatureNum; ++k) {
//                     int group = Feature.group[k];
//                     Bitboard rotatedP = recodeEx.playerRotatedBB[Feature.rotationType[k]];
//                     Bitboard rotatedO = recodeEx.opponentRotatedBB[Feature.rotationType[k]];

//                     int pattern = convert(pext(rotatedP, Feature.masks[group]), pext(rotatedO, Feature.masks[group]));
//                     occurrence[group].insert(pattern);
//                 }
//             }
//         }

//         for (int j = 0; j < GroupNum; ++j) {
//             int numAllPattern = integerPow(3, popcount(Feature.masks[j]));
//             std::cout << "Stage " << i << ", Group " << j << ": " << ((double)occurrence[j].size() / (double)numAllPattern * 100.0) << "%" << std::endl;
//         }
//     }
// }