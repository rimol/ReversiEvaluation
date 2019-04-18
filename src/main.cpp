#include <algorithm>
#include <iostream>
#include <string>
#include "board.h"
#include "bitboard.h"
#include "eval.h"
#include "evalgen.h"
#include "engine.h"
#include "recgen.h"

void doReversi() {
	// 評価関数フォルダを指定
	std::string folderPath;
	std::cout << "Enter a folder path that includes eval files:";
	std::cin >> folderPath;

	evalValuesFolderPath = folderPath;

	// コンピュータは白
	Bitboard p = 0x0000000810000000ULL;
	Bitboard o = 0x0000001008000000ULL;
	Color current = Black;
	bool passed = false;
	while (true) {
		Bitboard moves = getMoves(p, o);
		if (moves == 0ULL) {
			std::cout << "Pass" << std::endl;
			if (passed) {
				std::cout << "The game is over" << std::endl;
				break;
			}
			else {
				passed = true;
			}
		}
		else {
			// 盤面を表示する
			for (int i = 63; i >= 0; --i) {
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

			// 入力を待つ
			int sq;
			if (current == Black) {
				std::cout << "choose your next move from: ";
				Bitboard t(moves);
				while (t) {
					Bitboard b = t & -t;
					std::cout << tzcnt(b);
					t ^= b;
					std::cout << (t ? ", " : "\n");
				}
				std::cin >> sq;
			}
			else sq = chooseBestMove(p, o);

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
 		else if (command == "exit") break; 
	}

	return 0;
}