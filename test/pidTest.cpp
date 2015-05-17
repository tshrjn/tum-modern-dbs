#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>
#include <unordered_map>

#include "buffer/PID.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	PID pid1(1, 2);
	assert(pid1.getSegment() == 1);
	assert(pid1.getPage() == 2);
	assert("(1,2)" == (string)pid1);

	uint16_t maxSegment = pow(2, 16) - 1;
	uint64_t maxPage = pow(2, 48) - 1;
	PID pid3(maxSegment, maxPage);

	assert(~pid3.getValue() == 0);
	assert(pid3.getSegment() == maxSegment);
	assert(pid3.getPage() == maxPage);
	
	std::cout << "Test successfull" << std::endl;


}