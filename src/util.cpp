#include <time.h>
#include "util.h"

#ifdef _WIN32
#include <direct.h>
constexpr char PathDivider = '\\';

bool makeFolder(std::string folderPath) {
    return _mkdir(folderPath.c_str()) == 0;
}
#elif __APPLE__
#include <sys/stat.h>
constexpr char PathDivider = '/';

bool makeFolder(std::string folderPath) {
    return mkdir(folderPath.c_str(), 0777) == 0;
}
#endif

std::string createCurrentTimeFolderIn(std::string folderPath) {
    if (folderPath.back() != PathDivider) {
        folderPath += PathDivider;
    }
    folderPath += std::to_string(time(NULL));
    makeFolder(folderPath);
    return folderPath;
}

std::string addFileNameAtEnd(std::string folderPath, std::string fileNameNoExtention, std::string extention) {
    if (folderPath.back() != PathDivider) {
        folderPath += PathDivider;
    }
    return folderPath + fileNameNoExtention + '.' + extention;
}