#include <iostream>
#include <string>
#include "recgen.h"
 
int main() {
	int N;
	std::string folderPath;
	std::cout << "Enter N: ";
	std::cin >> N;
	std::cout << "Enter a folder path: ";
	std::cin >> folderPath;

	generateRecode(N, folderPath);

	std::cout << std::endl << "completed!" << std::endl << "Press any key to finish this program:";
	
	char c;
	std::cin >> c;

	return 0;
}