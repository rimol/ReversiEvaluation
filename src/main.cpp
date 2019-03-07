#include <iostream>
#include <string>
#include "test.h"
 
int main() {
	testBitboardOperation();

	std::cout << std::endl << "completed!" << std::endl << "Press any key to finish this program:";
	
	char c;
	std::cin >> c;

	return 0;
}