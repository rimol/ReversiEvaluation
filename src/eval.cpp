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
    Bitboard playerRotatedBB[8], opponentRotatedBB[8];
    rotateAndFlipBB(p, playerRotatedBB);
    rotateAndFlipBB(o, opponentRotatedBB);

    int t = (popcount(p | o) - 4 - 1) / StageInterval;

    double e = 0.0;
    for (int i = 0; i < FeatureNum; ++i) {
        int g = Feature.group[i];
        Bitboard p_ = playerRotatedBB[Feature.rotationType[i]];
        Bitboard o_ = opponentRotatedBB[Feature.rotationType[i]];

        e += evaluationValues[t][g][extract(p_, o_, g)];
    }

    return e;
}