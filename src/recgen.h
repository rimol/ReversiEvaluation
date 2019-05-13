#pragma once
#include <filesystem>
#include <string>
// n回自動対戦をして、各手数における盤面+最終石差の記録をファイルに保存
void generateRecode(int n, int depth, std::filesystem::path folderPath);