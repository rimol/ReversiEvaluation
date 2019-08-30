#include "pattern.h"
#include "record.h"
#include <string>
#include <vector>

class EvalGen {
    int numStages;
    int stageInterval;
    const Pattern &usedPattern;
    std::string patternName;

    // 各特徴の評価値、評価値の更新分、ステップサイズをまとめて持つ
    struct FeatureValue {
        double weight = 0.0;
        double update = 0.0;
        double stepSize = 0.0;
    };

    // 計算用配列
    FeatureValue **featureValues;

    void clearFeatureValues();
    inline double evalLoss(const RecordEx &recordEx);
    inline void applyUpdatesOfEvalValues();
    // 評価値を計算してファイルに保存し、実際の結果と最終的な評価値による予測値の分散を返す。
    double calculateEvaluationValue(const std::vector<std::string> &recordFilepaths);

public:
    void run(const std::string &recordsFolderPath, const std::string &outputFolderPath, int first, int last);

    EvalGen(int numStages, const std::string &patternName);
    ~EvalGen();
};

// 各ステージごとに出現するパターンの種類の多さを表示する
// void printPatternCoverage(const std::string &recordsFolderPath);