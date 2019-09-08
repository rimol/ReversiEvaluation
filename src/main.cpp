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

    PatternEvaluator evaluator(evalValuesFolderPath);
    NegaScoutEngine engine(evaluator);

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
                auto movesWithScore = engine.evalAllMoves(reversi.p, reversi.o, depth);
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
    int n, randomDepth, searchDepth, exactDepth;
    std::string saveFolderpath, evalFolderpath;

    std::cout << "The number of records you want to generate: ";
    std::cin >> n;
    std::cout << "Random opening depth: ";
    std::cin >> randomDepth;
    std::cout << "Searching(Mid game) depth: ";
    std::cin >> searchDepth;
    std::cout << "Exact depth: ";
    std::cin >> exactDepth;
    std::cout << "Save Folder: ";
    std::cin >> saveFolderpath;
    std::cout << "Folder where evaluation files you want to use exist: ";
    std::cin >> evalFolderpath;

    generateRecords(n, randomDepth, searchDepth, exactDepth, evalFolderpath, saveFolderpath);

    std::cout << "Done!\n";
}

void doEvalGen() {
    std::string patternName, trainingDataFolderPath, outputFolderPath;
    int numStages, first, last;
    std::cout << "Pattern name:";
    std::cin >> patternName;
    std::cout << "The number of stages:";
    std::cin >> numStages;
    std::cout << "Training Data Folder Path:";
    std::cin >> trainingDataFolderPath;
    std::cout << "Enter a folder path where you want to save the data:";
    std::cin >> outputFolderPath;
    std::cout << "first: ";
    std::cin >> first;
    std::cout << "last: ";
    std::cin >> last;

    // printPatternCoverage(recordsFolderPath);

    if (first <= last) {
        EvalGen evalGen(numStages, patternName);
        evalGen.run(trainingDataFolderPath, outputFolderPath, first, last);
    }

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

    play(N, depth, evalValuesFolderPath);
}

void doDatabaseConversion() {
    std::string databaseFolderPath, outputFolderPath;

    std::cout << "Enter a folder path that includes database files:";
    std::cin >> databaseFolderPath;
    std::cout << "Enter an output folder path:";
    std::cin >> outputFolderPath;

    convertDatabaseToRecords(databaseFolderPath, outputFolderPath);
}

void doTrainingDataMerge() {
    std::vector<std::string> inputFolderpaths;
    std::string outputFolderpath;

    while (true) {
        std::string folderpath;
        std::cout << "Enter a folder path where training data is stored. when you finish entering folder paths, enter 0:";
        std::cin >> folderpath;

        if (folderpath == "0")
            break;

        inputFolderpaths.push_back(folderpath);
    }

    std::cout << "Enter a folder path where you want to save results:";
    std::cin >> outputFolderpath;

    mergeTrainingDataFiles(inputFolderpaths, outputFolderpath);

    std::cout << "Done!" << std::endl;
}

// void doRecFix() {
//     std::string inputFolderpath, outputFolderpath;
//     std::cout << "Enter a folder path where old-type records are stored:";
//     std::cin >> inputFolderpath;
//     std::cout << "Enter a folder path where you want to save data:";
//     std::cin >> outputFolderpath;

//     fixRecords(inputFolderpath, outputFolderpath);
//     std::cout << "Done!" << std::endl;
// }

void doTrainingDataGen() {
    std::string recordFolderpath, saveFolderpath;
    std::cout << "A folder path to .txt records:";
    std::cin >> recordFolderpath;
    std::cout << "Save Folder:";
    std::cin >> saveFolderpath;

    generateTrainingData(recordFolderpath, saveFolderpath);
    std::cout << "Done!" << std::endl;
}

void doEvalComp() {
    int n;
    std::string weightFolderpath1, weightFolderpath2;

    std::cout << "How many plays you want to make?:";
    std::cin >> n;
    std::cout << "weight folder path(1):";
    std::cin >> weightFolderpath1;
    std::cout << "weight folder path(2):";
    std::cin >> weightFolderpath2;

    runSelfPlay(n, weightFolderpath1, weightFolderpath2);
    std::cout << "Done!" << std::endl;
}

void doCorrectRecords() {
    int exactDepth;
    std::string recordFolderpath, saveFolderpath, weightFolderpath;
    std::cout << "A folder path to .txt records:";
    std::cin >> recordFolderpath;
    std::cout << "Save Folder:";
    std::cin >> saveFolderpath;
    std::cout << "Exact Depth:";
    std::cin >> exactDepth;
    std::cout << "weight foler path(used by solver):";
    std::cin >> weightFolderpath;

    PatternEvaluator evaluator(weightFolderpath);
    Solver solver(evaluator);

    correctRecords(recordFolderpath, saveFolderpath, exactDepth, solver);
}

void doEvalCompress() {
    std::string weightFolderpath, saveFolderpath;
    std::cout << "Weight Folder Path:";
    std::cin >> weightFolderpath;
    std::cout << "Save Folder Path:";
    std::cin >> saveFolderpath;

    compressEvalFiles(weightFolderpath, saveFolderpath);
}

int main() {
    initToBase3();
    initCountFlipTables();
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
        else if (command == "evalcomp")
            doEvalComp();
        else if (command == "mergetrain")
            doTrainingDataMerge();
        else if (command == "corec")
            doCorrectRecords();
        // else if (command == "fixrec")
        //     doRecFix();
        else if (command == "traingen")
            doTrainingDataGen();
        else if (command == "compresseval")
            doEvalCompress();
        else if (command == "exit")
            break;
    }

    return 0;
}