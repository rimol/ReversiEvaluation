#include <string>

// 各ステージごとに出現するパターンの種類の多さを表示する
void printPatternCoverage(const std::string &recordsFolderPath);
void generateEvaluationFiles(std::string recordsFolderPath, std::string outputFolderPath, int first, int last);