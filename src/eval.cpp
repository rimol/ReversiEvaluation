#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "bitboard.h"
#include "eval.h"

static double evaluationValues[60][GroupNum][EvalArrayLength];
static double mobilityWeight[60];
static double intercept[60];

void loadEvalValues(std::filesystem::path evalValuesFolderPath) {
    for (int i = 0; i < 60; ++i) {
        std::ifstream ifs(evalValuesFolderPath / (std::to_string(i) + ".bin"), std::ios::binary);
        ifs.read((char*)evaluationValues[i], sizeof(double) * GroupNum * EvalArrayLength);
        ifs.read((char*)&mobilityWeight[i], sizeof(double));
        ifs.read((char*)&intercept[i], sizeof(double));
    }
}

double evaluate(Bitboard p, Bitboard o) {
    Bitboard p_r[4] = { p, rotateRightBy90(p), rotateBy180(p), rotateBy180(rotateRightBy90(p)) };
    Bitboard o_r[4] = { o, rotateRightBy90(o), rotateBy180(o), rotateBy180(rotateRightBy90(o)) };
    int t = popcount(p | o) - 4 - 1;
    double e = intercept[t];
    for (int i = 0; i < FeatureNum; ++i) {
        int g = Feature.group[i];
        Bitboard p_ = p_r[Feature.rotationType[i]];
        Bitboard o_ = o_r[Feature.rotationType[i]];

        e += evaluationValues[t][g][extract(p_, o_, g)];
    }
    
    return e + (double)getMobility(p, o) * mobilityWeight[t];
}