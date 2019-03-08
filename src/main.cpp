#include <iostream>
#include <string>
#include "recgen.h"
 
int main() {
	int N, depth;
	std::string folderPath;
	std::cout << "Enter N: ";
	std::cin >> N;
	std::cout << "Enter a depth:";
	std::cin >> depth;
	std::cout << "Enter a folder path: ";
	std::cin >> folderPath;

	generateRecode(N, depth, folderPath);

	std::cout << std::endl << "completed!" << std::endl << "Press any key to finish this program:";
	
	char c;
	std::cin >> c;

	return 0;
}