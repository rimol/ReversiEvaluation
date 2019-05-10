#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "bitboard.h"
#include "eval.h"

static double evaluationValues[60][FeatureNum][6561];
static double mobilityWeight[60];
static double intercept[60];

void loadEvalValues(std::string evalValuesFolderPath) {
    if (evalValuesFolderPath.back() != '\\') evalValuesFolderPath += '\\';
    for (int i = 0; i < 60; ++i) {
        std::ifstream ifs(evalValuesFolderPath + std::to_string(i) + ".bin", std::ios::binary);
        ifs.read((char*)evaluationValues[i], sizeof(double) * FeatureNum * 6561);
        ifs.read((char*)&mobilityWeight[i], sizeof(double));
        ifs.read((char*)&intercept[i], sizeof(double));
    }
}

double evaluate(Bitboard p, Bitboard o) {
    int t = popcount(p | o) - 4 - 1;
    double e = intercept[t];
    for (int i = 0; i < FeatureNum; ++i) {
        e += evaluationValues[t][i][extract(p, o, i)];
    }
    return e + (double)getMobility(p, o) * mobilityWeight[t];
}