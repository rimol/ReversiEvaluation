#include "eval.h"
#include "bitboard.h"
#include "pattern.h"
#include "util.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using std::cerr;
using std::endl;

// TODO: パターンの種類によってMaxNumPatternのところを変える
// symmetricなパターンを詰める
PatternEvaluator::PatternEvaluator(const std::string &weightFolderpath) {
    auto infoFilepath = addFileNameAtEnd(weightFolderpath, "info", "txt");
    std::ifstream ifs(infoFilepath);

    std::string patternName;
    ifs >> patternName;
    usedPattern = &Patterns.at(patternName);

    ifs >> numStages;
    assert(60 % numStages == 0);
    stageInterval = 60 / numStages;

    ifs.close();

    weights = new double **[numStages];
    for (int i = 0; i < numStages; ++i) {
        weights[i] = new double *[usedPattern->numGroup()];
        for (int j = 0; j < usedPattern->numGroup(); ++j) {
            weights[i][j] = new double[usedPattern->numPackedIndex(j)];
        }
    }

    for (int i = 0; i < numStages; ++i) {
        ifs.open(addFileNameAtEnd(weightFolderpath, std::to_string(i + 1), "bin"), std::ios::binary);
        for (int j = 0; j < usedPattern->numGroup(); ++j) {
            ifs.read((char *)weights[i][j], sizeof(double) * usedPattern->numPackedIndex(j));
        }
        ifs.close();
    }
}

PatternEvaluator::~PatternEvaluator() {
    for (int i = 0; i < numStages; ++i) {
        for (int j = 0; j < usedPattern->numGroup(); ++j) {
            delete[] weights[i][j];
        }
        delete[] weights[i];
    }
    delete[] weights;
}

double PatternEvaluator::evaluate(Bitboard p, Bitboard o) const {
    if (popcount(p) == 0)
        return -popcount(o);

    if (popcount(o) == 0)
        return popcount(p);

    Bitboard playerRotatedBB[8], opponentRotatedBB[8];
    rotateAndFlipBB(p, playerRotatedBB);
    rotateAndFlipBB(o, opponentRotatedBB);

    int t = getStage(p, o);

    double e = 0.0;
    for (int i = 0; i < usedPattern->numPattern(); ++i) {
        int group = usedPattern->group(i);
        Bitboard p_ = playerRotatedBB[usedPattern->rotationType(i)];
        Bitboard o_ = opponentRotatedBB[usedPattern->rotationType(i)];

        e += weights[t][group][usedPattern->extract(p_, o_, group)];
    }

    return e;
}

double ClassicEvaluator::evaluate(Bitboard p, Bitboard o) const {
    double squareValue[] = {
        30, -12, 0, -1, -1, 0, -12, 30,     // これはコード補完で
        -12, -15, -3, -3, -3, -3, -15, -12, // 左の配列が縦に展開されないように
        0, -3, 0, -1, -1, 0, -3, 0,         // するためのテクニック（？）
        -1, -3, -1, -1, -1, -1, -3, -1,     //
        -1, -3, -1, -1, -1, -1, -3, -1,     //
        0, -3, 0, -1, -1, 0, -3, 0,         //
        -12, -15, -3, -3, -3, -3, -15, -12, //
        30, -12, 0, -1, -1, 0, -12, 30,     //
    };

    Bitboard occupancy = p | o;
    if (occupancy >> 0 & 1ULL) {
        squareValue[1] *= -1;
        squareValue[8] *= -1;
        squareValue[9] *= -1;
    }

    if (occupancy >> 7 & 1ULL) {
        squareValue[6] *= -1;
        squareValue[14] *= -1;
        squareValue[15] *= -1;
    }

    if (occupancy >> 56 & 1ULL) {
        squareValue[48] *= -1;
        squareValue[49] *= -1;
        squareValue[57] *= -1;
    }

    if (occupancy >> 63 & 1ULL) {
        squareValue[54] *= -1;
        squareValue[55] *= -1;
        squareValue[62] *= -1;
    }

    const double weightNumStone[15] = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.4, 0.4, 1, 3};
    const double weightMobility[15] = {0.4, 1.2, 1.3, 1.1, 1, 1, 1, 0.8, 0.7, 0.6, 0.4, 0.4, 0.3, 0, 0};
    const double weightStability[15] = {1, 1, 1, 2, 2, 2, 2, 2, 2, 2.5, 2.5, 2.5, 3, 3, 3};
    const double weightSquareValue[15] = {1, 1, 1, 1, 1, 1, 1, 1, 0.8, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4};

    int numP = popcount(p);
    int numO = popcount(o);

    if (numP == 0)
        return -999999;

    if (numO == 0)
        return 999999;

    Bitboard stable = getStableStones(p, o);

    int stabilityP = popcount(p & stable);
    int stabilityO = popcount(o & stable);

    int mobilityP = getWeightedMobility(p, o);
    int mobilityO = getWeightedMobility(o, p);

    int valueP = 0, valueO = 0;
    for (int i = 0; i < 64; ++i) {
        if (stable >> i & 1ULL)
            continue;

        if (p >> i & 1ULL) {
            if (stable >> i & 1ULL) {
                valueP += std::max(squareValue[i], 0.0);
            } else
                valueP += squareValue[i];
        } else if (o >> i & 1ULL) {
            if (stable >> i & 1ULL) {
                valueO += std::max(squareValue[i], 0.0);
            } else
                valueO += squareValue[i];
        }
    }

    int stage = (popcount(p | o) - 4 - 1) / 4;
    return weightNumStone[stage] * (numP - numO) + weightSquareValue[stage] * (valueP - valueO) + weightMobility[stage] * (mobilityP - mobilityO) + weightStability[stage] * (stabilityP - stabilityO);
}