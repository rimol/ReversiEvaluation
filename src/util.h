#pragma once
#include <string>
#include <vector>

std::string createCurrentTimeFolderIn(std::string folderPath);
std::string addFileNameAtEnd(std::string folderPath, std::string fileNameNoExtention, std::string extention);
void enumerateFilesIn(std::string folderPath, std::vector<std::string>& out);