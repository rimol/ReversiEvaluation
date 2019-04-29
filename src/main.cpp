#include <algorithm>
#include <iostream>
#include <string>
#include "bitboard.h"
#include "eval.h"
#include "evalgen.h"
#include "engine.h"
#include "recgen.h"
#include "solver.h"

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

	Color human;
	int depth;
	std::cout << "Choose your stone color from black(0) or white(1):";
	std::cin >> human;
	std::cout << "Enter the search depth:";
	std::cin >> depth;
	std::cout << "Enter a folder path that includes eval files:";
	std::cin >> evalValuesFolderPath;

	bool passed = false;
	Color current = Black;
	Bitboard p = 0x0000000810000000ULL;
	Bitboard o = 0x0000001008000000ULL;
	while (true) {
		Bitboard moves = getMoves(p, o);
		if (moves == 0ULL) {
			if (passed) {
				std::cout << "The game is over." << std::endl;
				break;
			}
			else {
				std::cout << "Pass" << std::endl;
				passed = true;
			}
		}
		else {
			// 盤面を表示する
			std::cout << " ABCDEFGH" << std::endl;
			for (int i = 63; i >= 0; --i) {
				if (i % 8 == 7) std::cout << (8 - i / 8);

        		if (p >> i & 1ULL) {
            		std::cout << (current == Black ? 'X' : 'O');
        		}
        		else if (o >> i & 1ULL) {
            		std::cout << (current == Black ? 'O' : 'X');
        		}
        		else if (moves >> i & 1ULL) {
            		std::cout << "*";
        		}
				else {
            		std::cout << "-";
				}

        		if (i % 8 == 0) std::cout << std::endl;
    		}
			std::cout << "p:" << (current == Black ? "Black " : "White ") << "p: " << popcount(p) << " o: " << popcount(o) << std::endl;

			// 入力を待つ
			int sq;
			if (current == human) {
				std::string pos;
				std::cin >> pos;
				sq = AlphabetToNumber[pos[0]] + (7 - (pos[1] - '1')) * 8;
			}
			else sq = chooseBestMove(p, o, depth);

			Bitboard sqbit = 1ULL << sq;
			Bitboard flip = getFlip(p, o, sqbit);
			p ^= flip | sqbit;
			o ^= flip;
		}

		std::swap(p, o);
		current = ~current;
	}
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

int main() {
	while (true) {
		std::cout << "Enter a command:";
		std::string command;
		std::cin >> command;

		if (command == "reversi") doReversi();
		else if (command == "recgen") doRecGen();
		else if (command == "evalgen") doEvalGen();
		else if (command == "ffo") ffotest();
 		else if (command == "exit") break; 
	}

	return 0;
}