#include <filesystem>
#include <string>

// 棋譜が入っているフォルダ（recgenで生成されるもの）を指定する
void generateEvaluationFiles(std::filesystem::path recodesFolderPath, std::filesystem::path outputFolderPath, double beta);