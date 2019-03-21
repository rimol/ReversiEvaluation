#include <iostream>
#include <string>
#include "evalgen.h"
 
int main() {
	std::string recodesFolderPath;
	std::string outputFolderPath;
	std::cout << "Enter a folder path which it has recodes: ";
	std::cin >> recodesFolderPath;
	std::cout << "Enter a folder path where you want to save evaluation files:";
	std::cin >> outputFolderPath;

	generateEvaluationFiles(recodesFolderPath, outputFolderPath);

	std::cout << std::endl << "completed!" << std::endl << "Press any key to finish this program:";
	
	char c;
	std::cin >> c;

	return 0;
}