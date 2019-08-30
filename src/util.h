#pragma once
#include <string>
#include <vector>

#ifdef _WIN32
constexpr char PathDivider = '\\';
#elif __APPLE__
constexpr char PathDivider = '/';
#endif

std::string createCurrentTimeFolderIn(std::string folderPath);
std::string addFileNameAtEnd(std::string folderPath, std::string fileNameNoExtention, std::string extention);
void enumerateFilesIn(std::string folderPath, std::vector<std::string> &out);