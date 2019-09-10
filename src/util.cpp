#include "util.h"
#include <time.h>

std::string getExtension(const std::string &filepath) {
    int begin = filepath.find_last_of('.') + 1;
    return filepath.substr(begin, filepath.size() - begin);
}

std::string getFilenameNoExtension(const std::string &filepath) {
    auto begin = filepath.find_last_of(PathDivider) + 1;
    auto end = filepath.find_last_of('.');
    return filepath.substr(begin, end - begin);
}

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
bool makeFolder(std::string folderPath) {
    return _mkdir(folderPath.c_str()) == 0;
}
#elif __APPLE__
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
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
void enumerateFilesIn(std::string folderPath, std::vector<std::string> &out) {
    if (folderPath.back() != PathDivider) {
        folderPath += PathDivider;
    }
#ifdef _WIN32
    WIN32_FIND_DATA win32fd;
    HANDLE hFind = FindFirstFile((folderPath + "*.*").c_str(), &win32fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    do {
        if (win32fd.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY) {
            std::string filename = (const char *)win32fd.cFileName;
            out.push_back(folderPath + filename);
        }
    } while (FindNextFile(hFind, &win32fd));
    FindClose(hFind);
#elif __APPLE__
    auto dir = opendir(folderPath.c_str());
    if (dir == NULL)
        return;

    dirent *dent;
    do {
        dent = readdir(dir);
        if (dent != NULL) {
            out.push_back(folderPath + dent->d_name);
        }
    } while (dent != NULL);

    closedir(dir);
#endif
}

StopWatch::StopWatch() : start(std::chrono::system_clock::now()) {}

void StopWatch::setTimePoint() {
    timePoints.push_back(std::chrono::system_clock::now());
}

long long StopWatch::getElapsedTime_millisec(int i) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(timePoints[i] - start).count();
}