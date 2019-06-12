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
#include <sys/types.h>
#include <dirent.h>
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

/*
    名前が'.', '..'のファイルがなぜか混じります気をつけて
 */
void enumerateFilesIn(std::string folderPath, std::vector<std::string>& out) {
    #ifdef _WIN32
    // 実装してください
    #elif __APPLE__
    if (folderPath.back() != PathDivider) {
        folderPath += PathDivider;
    }

    auto dir = opendir(folderPath.c_str());
    if (dir == NULL) return;

    dirent* dent;
    do {
        dent = readdir(dir);
        if (dent != NULL) {
            out.push_back(folderPath + dent->d_name);
        }
    } while(dent != NULL);

    closedir(dir);
    #endif
}