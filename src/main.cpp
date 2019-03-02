#include <iostream>
#include <string>
#include "recgen.h"
 
int main() {
	std::string folderPath;
	std::cout << "Enter a folder path:";
	std::cin >> folderPath;

	generateRecode(1, folderPath);

	std::cout << std::endl << "completed!" << std::endl << "Press any key to finish this program:";
	
	char c;
	std::cin >> c;

	return 0;
}