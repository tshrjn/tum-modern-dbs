#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>
#include <unordered_map>

#include "segment/TID.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	TID tid1(1, 2);
	assert(tid1.getPage() == 1);
	assert(tid1.getSlot() == 2);
	assert("(1,2)" == (string)tid1);

	uint64_t maxPage = pow(2, 48) - 1;
	uint16_t maxSlot = pow(2, 16) - 1;
	TID tid3(maxPage, maxSlot);

	assert(~tid3.getValue() == 0);
	assert(tid3.getPage() == maxPage);
	assert(tid3.getSlot() == maxSlot);

	std::cout << "Test successfull" << std::endl;
}