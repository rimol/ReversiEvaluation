#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "bitboard.h"
#include "eval.h"

std::string evalValuesFolderPath = "";

// この配列たちは、評価値生成のときも使うのでグローバルにしています。
double horizontal[8][6561];
double vertical[8][6561];
double corner[4][6561];
double mobility = 0;

void clearArrays() {
    std::fill((double*)horizontal, (double*)(horizontal + 8), 0);
    std::fill((double*)vertical, (double*)(vertical + 8), 0);
    std::fill((double*)corner, (double*)(corner + 4), 0);    
    mobility = 0;
}

void changeEvaluationTables(int t) {
    std::stringstream ss;
    ss << evalValuesFolderPath;
    if (evalValuesFolderPath.back() != '\\') ss << '\\';
    ss << t << ".bin";

    std::ifstream ifs(ss.str(), std::ios::binary);
    ifs.read((char*)horizontal, sizeof(double) * 8 * 6561);
    ifs.read((char*)vertical, sizeof(double) * 8 * 6561);
    ifs.read((char*)corner, sizeof(double) * 4 * 6561);
    ifs.read((char*)&mobility, sizeof(double));
}

double evaluate(Bitboard p, Bitboard o) {
    double e = 0;

    e += horizontal[0][extractHorizontal(0, p, o)];
    e += horizontal[1][extractHorizontal(1, p, o)];
    e += horizontal[2][extractHorizontal(2, p, o)];
    e += horizontal[3][extractHorizontal(3, p, o)];
    e += horizontal[4][extractHorizontal(4, p, o)];
    e += horizontal[5][extractHorizontal(5, p, o)];
    e += horizontal[6][extractHorizontal(6, p, o)];
    e += horizontal[7][extractHorizontal(7, p, o)];

    e += vertical[0][extractVertical(0, p, o)];
    e += vertical[1][extractVertical(1, p, o)];
    e += vertical[2][extractVertical(2, p, o)];
    e += vertical[3][extractVertical(3, p, o)];
    e += vertical[4][extractVertical(4, p, o)];
    e += vertical[5][extractVertical(5, p, o)];
    e += vertical[6][extractVertical(6, p, o)];
    e += vertical[7][extractVertical(7, p, o)];
    
    e += corner[0][extractCorner(0, p, o)];
    e += corner[1][extractCorner(1, p, o)];
    e += corner[2][extractCorner(2, p, o)];
    e += corner[3][extractCorner(3, p, o)];

    e += (double)getMobility(p, o) * mobility;

    return e;
}