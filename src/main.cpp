#include "bitboard.h"
#include "converter.h"
#include "engine.h"
#include "eval.h"
#include "evalgen.h"
#include "recgen.h"
#include "reversi.h"
#include "solver.h"
#include "test.h"
#include "util.h"
#include <algorithm>
#include <iostream>
#include <string>

void doReversi() {
    int AlphabetToNumber[1 << 8];
    AlphabetToNumber['a'] = 7;
    AlphabetToNumber['b'] = 6;
    AlphabetToNumber['c'] = 5;
    AlphabetToNumber['d'] = 4;
    AlphabetToNumber['e'] = 3;
    AlphabetToNumber['f'] = 2;
    AlphabetToNumber['g'] = 1;
    AlphabetToNumber['h'] = 0;

    Reversi reversi;
    Color human;
    int depth;
    std::string evalValuesFolderPath;
    std::cout << "Choose your stone color from black(0) or white(1):";
    std::cin >> human;
    std::cout << "Enter the search depth:";
    std::cin >> depth;
    std::cout << "Enter a folder path that includes eval files:";
    std::cin >> evalValuesFolderPath;

    loadEvalValues(evalValuesFolderPath);

    while (!reversi.isFinished) {
        reversi.print();

        bool moved = false;
        while (!moved) {
            int sq;
            if (reversi.c == human) {
                std::string pos;
                std::cin >> pos;
                sq = AlphabetToNumber[pos[0]] + (7 - (pos[1] - '1')) * 8;
            } else {
                auto movesWithScore = evalAllMoves(reversi.p, reversi.o, depth);
                double bestScore = -100000.0;
                for (auto &ms : movesWithScore) {
                    std::cout << convertToLegibleSQ(ms.move) << ": " << ms.score << std::endl;
                    if (ms.score > bestScore) {
                        sq = ms.move;
                        bestScore = ms.score;
                    }
                }
                // コンピュータが選択した手を表示する
                std::cout << "Computer selected: " << convertToLegibleSQ(sq) << std::endl;
            }

            moved = reversi.move(sq);
            if (!moved)
                std::cout << "the selected move is a illegal move. select again:";
        }
    }

    reversi.print();
    std::cout << "The game is over." << std::endl;
}

void doRecGen() {
    int n, depth;
    std::string folderPath;
    std::cout << "Specify the number of records you want to generate:";
    std::cin >> n;
    std::cout << "Specify a depth of the exact play:";
    std::cin >> depth;
    std::cout << "Enter a folder path where you want to create the save folder:";
    std::cin >> folderPath;

    generateRecord(n, depth, folderPath);

    std::cout << "Done!\n";
}

void doEvalGen() {
    std::string recordsFolderPath, outputFolderPath;
    int first, last;
    std::cout << "Enter a folder path where records are stored:";
    std::cin >> recordsFolderPath;
    std::cout << "Enter a folder path where you want to save the data:";
    std::cin >> outputFolderPath;
    std::cout << "first: ";
    std::cin >> first;
    std::cout << "last: ";
    std::cin >> last;

    printPatternCoverage(recordsFolderPath);

    if (first <= last)
        generateEvaluationFiles(recordsFolderPath, outputFolderPath, first, last);

    std::cout << "Done!" << std::endl;
}

void doAutoPlay() {
    int N;
    int depth;
    std::string evalValuesFolderPath;
    std::cout << "Enter the number of plays you want to make:";
    std::cin >> N;
    std::cout << "Enter the search depth:";
    std::cin >> depth;
    std::cout << "Enter a folder path that includes eval files:";
    std::cin >> evalValuesFolderPath;

    loadEvalValues(evalValuesFolderPath);

    play(N, depth);
}

void doDatabaseConversion() {
    std::string databaseFolderPath, outputFolderPath;

    std::cout << "Enter a folder path that includes database files:";
    std::cin >> databaseFolderPath;
    std::cout << "Enter an output folder path:";
    std::cin >> outputFolderPath;

    convertDatabaseToRecord(databaseFolderPath, outputFolderPath);
}

void doRecordMerge() {
    std::vector<std::string> inputFolderpaths;
    std::string outputFolderpath;

    while (true) {
        std::string folderpath;
        std::cout << "Enter a folder path where records are stored. when you finish entering folder paths, enter 0:";
        std::cin >> folderpath;

        if (folderpath == "0")
            break;

        inputFolderpaths.push_back(folderpath);
    }

    std::cout << "Enter a folder path where you want to save results:";
    std::cin >> outputFolderpath;

    mergeRecordFiles(inputFolderpaths, outputFolderpath);

    std::cout << "Done!" << std::endl;
}

void doRecFix() {
    std::string inputFolderpath, outputFolderpath;
    std::cout << "Enter a folder path where old-type records are stored:";
    std::cin >> inputFolderpath;
    std::cout << "Enter a folder path where you want to save data:";
    std::cin >> outputFolderpath;

    fixRecords(inputFolderpath, outputFolderpath);
    std::cout << "Done!" << std::endl;
}

int main() {
    initSymmetricPattern();
    // test();
    while (true) {
        std::cout << "Enter a command:";
        std::string command;
        std::cin >> command;

        if (command == "reversi")
            doReversi();
        else if (command == "recgen")
            doRecGen();
        else if (command == "evalgen")
            doEvalGen();
        else if (command == "ffo")
            ffotest();
        else if (command == "autoplay")
            doAutoPlay();
        else if (command == "conv")
            doDatabaseConversion();
        else if (command == "mergerec")
            doRecordMerge();
        else if (command == "fixrec")
            doRecFix();
        else if (command == "exit")
            break;
    }

    return 0;
}