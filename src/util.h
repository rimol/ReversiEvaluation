#pragma once
#include <chrono>
#include <string>
#include <vector>

// .のあとを返す
std::string getExtension(const std::string &filepath);
std::string getFilenameNoExtension(const std::string &filepath);

#ifdef _WIN32
constexpr char PathDivider = '\\';
#elif __APPLE__
constexpr char PathDivider = '/';
#endif

std::string createCurrentTimeFolderIn(std::string folderPath);
std::string addFileNameAtEnd(std::string folderPath, std::string fileNameNoExtention, std::string extention);
void enumerateFilesIn(std::string folderPath, std::vector<std::string> &out);

// StopWatch
class StopWatch {
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    TimePoint start;
    std::vector<TimePoint> timePoints;

public:
    void setTimePoint();
    long long getElapsedTime_millisec(int i);
    StopWatch();
};