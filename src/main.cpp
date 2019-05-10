#include <algorithm>
#include <iostream>
#include <string>
#include "bitboard.h"
#include "eval.h"
#include "evalgen.h"
#include "engine.h"
#include "recgen.h"
#include "reversi.h"
#include "solver.h"
#include "test.h"

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

	while(!reversi.isFinished) {
		reversi.print();

		bool moved = false;
		while (!moved) {
			int sq;
			if (reversi.c == human) {
				std::string pos;
				std::cin >> pos;
				sq = AlphabetToNumber[pos[0]] + (7 - (pos[1] - '1')) * 8;
			}
			else {
				sq = chooseBestMove(reversi.p, reversi.o, depth);
				// コンピュータが選択した手を表示する
				std::cout << convertToLegibleSQ(sq) << std::endl;
			}

			moved = reversi.move(sq);
			if (!moved) std::cout << "the selected move is a illegal move. select again:";
		}
	}

	std::cout << "The game is over." << std::endl;
}

void doRecGen() {
	int n, depth;
	std::string folderPath;
	std::cout << "Specify the number of recodes you want to generate:";
	std::cin >> n;
	std::cout << "Specify a depth of the exact play:";
	std::cin >> depth;
	std::cout << "Enter a folder path where you want to create the save folder:";
	std::cin >> folderPath;

	generateRecode(n, depth, folderPath);

	std::cout << "Done!\n";
}

void doEvalGen() {
	std::string recodesFolderPath, outputFolderPath;
	double beta;
	std::cout << "Enter a folder path where recodes are stored:";
	std::cin >> recodesFolderPath;
	std::cout << "Enter a folder path where you want to save the data:";
	std::cin >> outputFolderPath;
	std::cout << "Enter the value of beta(this will determine the step size):";
	std::cin >> beta;

	generateEvaluationFiles(recodesFolderPath, outputFolderPath, beta);

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

int main() {
	while (true) {
		std::cout << "Enter a command:";
		std::string command;
		std::cin >> command;

		if (command == "reversi") doReversi();
		else if (command == "recgen") doRecGen();
		else if (command == "evalgen") doEvalGen();
		else if (command == "ffo") ffotest();
		else if (command == "autoplay") doAutoPlay();
 		else if (command == "exit") break; 
	}

	return 0;
}