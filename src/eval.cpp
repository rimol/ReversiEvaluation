#include "eval.h"
#include "bitboard.h"
#include "util.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int SymmetricPattern[GroupNum][EvalArrayLength];
void initSymmetricPattern() {
    int flipSQ[2][64] = {};

    for (int i = 0; i < 64; ++i) {
        int x = i % 8;
        int y = i / 8;
        flipSQ[0][i] = x + (7 - y) * 8;       // vertical
        flipSQ[1][i] = (7 - y) + (7 - x) * 8; // diagonal
    }

    // 特徴自体の対称性
    for (int i = 0; i < GroupNum; ++i) {
        Bitboard v = flipVertical(Feature.masks[i]);
        Bitboard d = flipDiagonalA8H1(Feature.masks[i]);
        if (Feature.masks[i] == v || Feature.masks[i] == d) {
            int flipType = Feature.masks[i] == v ? 0 : 1;
            for (int j = 0; j < integerPow(3, popcount(Feature.masks[i])); ++j) {
                int k = j;
                Bitboard m = Feature.masks[i];
                std::vector<std::pair<int, int>> digits;
                while (m != 0ULL) {
                    digits.push_back({flipSQ[flipType][tzcnt(m)], k % 3});
                    k /= 3;
                    m &= m - 1ULL;
                }
                std::sort(digits.begin(), digits.end(), std::greater<std::pair<int, int>>());
                int l = 0;
                for (int i = 0; i < digits.size(); ++i) {
                    l *= 3;
                    l += digits[i].second;
                }

                SymmetricPattern[i][j] = SymmetricPattern[i][l] = std::min(j, l);
            }
        } else {
            for (int j = 0; j < EvalArrayLength; ++j) {
                SymmetricPattern[i][j] = j;
            }
        }
    }
}

static double evaluationValues[NumStages][GroupNum][EvalArrayLength];

void loadEvalValues(std::string evalValuesFolderPath) {
    for (int i = 0; i < NumStages; ++i) {
        std::ifstream ifs(addFileNameAtEnd(evalValuesFolderPath, std::to_string(i + 1), "bin"), std::ios::binary);
        ifs.read((char *)evaluationValues[i], sizeof(double) * GroupNum * EvalArrayLength);
    }

    // 得点充填率を出力
    for (int i = 0; i < GroupNum; ++i) {
        for (int j = 0; j < NumStages; ++j) {
            int numAllPattern = integerPow(3, popcount(Feature.masks[i]));
            int numNonZeroPattern = 0;
            for (int k = 0; k < numAllPattern; ++k) {
                numNonZeroPattern += evaluationValues[j][i][k] != 0;
            }

            std::cout << "Stage " << j << ", Group " << i << ": " << ((double)numNonZeroPattern / (double)numAllPattern * 100.0) << std::endl;
        }
    }
}

double evaluate(Bitboard p, Bitboard o) {
    // これ、差を返すようにしたほうがいいかな...?
    if (popcount(p) == 0) {
        return -EvalInf;
    }

    if (popcount(o) == 0) {
        return EvalInf;
    }

    Bitboard playerRotatedBB[8], opponentRotatedBB[8];
    rotateAndFlipBB(p, playerRotatedBB);
    rotateAndFlipBB(o, opponentRotatedBB);

    int t = getStage(p, o);

    double e = 0.0;
    for (int i = 0; i < FeatureNum; ++i) {
        int g = Feature.group[i];
        Bitboard p_ = playerRotatedBB[Feature.rotationType[i]];
        Bitboard o_ = opponentRotatedBB[Feature.rotationType[i]];

        e += evaluationValues[t][g][extract(p_, o_, g)];
    }

    return e;
}

double evaluate_classic(Bitboard p, Bitboard o) {
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

    int stage = getStage(p, o);
    return weightNumStone[stage] * (numP - numO) + weightSquareValue[stage] * (valueP - valueO) + weightMobility[stage] * (mobilityP - mobilityO) + weightStability[stage] * (stabilityP - stabilityO);
}