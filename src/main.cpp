#include <iostream>
#include <string>
#include "recgen.h"
 
int main() {
	std::string folderPath;
	std::cout << "Enter a folder path:";
	std::cin >> folderPath;

	generateRecode(100, folderPath);

	std::cout << std::endl << "completed!" << std::endl;
	return 0;
}