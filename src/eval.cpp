#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "bitboard.h"
#include "eval.h"

std::string evalValuesFolderPath = "";

// 評価値生成のときも使うのでグローバルにしています。
double evaluationValues[FeatureNum][6561];
double mobilityWeight = 0;

void changeEvaluationTables(int t) {
    std::stringstream ss;
    ss << evalValuesFolderPath;
    if (evalValuesFolderPath.back() != '\\') ss << '\\';
    ss << t << ".bin";

    std::ifstream ifs(ss.str(), std::ios::binary);
    ifs.read((char*)evaluationValues, sizeof(double) * FeatureNum * 6561);
    ifs.read((char*)&mobilityWeight, sizeof(double));
}

double evaluate(Bitboard p, Bitboard o) {
    double e = 0;
    for (int i = 0; i < FeatureNum; ++i) {
        e += evaluationValues[i][extract(p, o, i)];
    }
    return e + (double)getMobility(p, o) * mobilityWeight;
}