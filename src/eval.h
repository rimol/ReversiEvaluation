#pragma once

#include "bitboard.h"
#include "pattern.h"
#include <cassert>
#include <string>

// 全敗勝ちとかAlphaBetaの最初の窓とかで使う。
// これぐらいのおおきさで十分。
constexpr double EvalInf = 1000.0;

class Evaluator {
public:
    virtual double evaluate(Bitboard p, Bitboard o) const = 0;
    virtual ~Evaluator() {}
};

// 古典評価関数
class ClassicEvaluator : public Evaluator {
public:
    double evaluate(Bitboard p, Bitboard o) const;
};

class PatternEvaluator : public Evaluator {
    int numStages;
    int stageInterval;
    const Pattern *usedPattern;
    double ***weights;

    inline int getStage(Bitboard p, Bitboard o) const {
        return (popcount(p | o) - 4 - 1) / stageInterval;
    }

public:
    double evaluate(Bitboard p, Bitboard o) const;
    // info.txtを読み込んでステージ数と使うパターンを判定し、それを元に評価値ファイルを読み込んで初期化
    PatternEvaluator(const std::string &weightFolderpath);
    ~PatternEvaluator();
};