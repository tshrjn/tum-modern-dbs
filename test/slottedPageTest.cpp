#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>

#include <unordered_map>

// #include "DBMS.hpp" // include your stuff here
#include "buffer/BufferManager.hpp"
#include "pages/SlottedPage.hpp"

using namespace std;

struct TestStruct1 {
	uint16_t a;
	uint16_t b;
	TestStruct1(uint16_t v_a, uint16_t v_b) : a(v_a), b(v_b) {}
};

struct TestStruct2 {
	uint32_t a;
	uint32_t b;
	TestStruct2(uint32_t v_a, uint32_t v_b) : a(v_a), b(v_b) {}
};

int main(int argc, char* argv[]) {
	SlottedPage sut1;
	std::cout << "SlottedPageTest: Testing init" << std::endl;
	assert(sut1.getCurrentFreeSpace() == SlottedPage::dataSize);
	assert(sut1.getCompactedFreeSpace() == SlottedPage::dataSize);

	/*
		Test perfect fitting
		Frame size is (4*4096) * 8bit
		sizeof(TestStruct1) = 2*16 bit = 32 bit
		sizeof(Header) = 64 bit
		sizeof(Slot) = 32 bit
		64bit + x * 32 bit + x * 32 bit = 4 * 4096 * 8 bit
		<=> x = 2047
		We can store exactly 2047 Teststructs in this test scenario on a given SlottedPage
	*/

	std::cout << "SlottedPageTest: Testing perfect fit with 2047 TestStructs(1)" << std::endl;
	for(int i = 0; i < 2047; i++) {
		auto slotId = sut1.allocateSlot(sizeof(TestStruct1));
		assert(slotId == i);
		sut1.storeData(slotId, (char*)(new TestStruct1(i, i/2)));
	}

	assert(!sut1.canAllocateSlot(sizeof(TestStruct1)));

	std::cout << "SlottedPageTest: 0 bytes should be free" << std::endl;
	assert(sut1.getCurrentFreeSpace() == 0);
	assert(sut1.getCompactedFreeSpace() == 0);
	std::cout << "SlottedPageTest: 0 bytes are free" << std::endl;
	std::cout << "SlottedPageTest: Test successfull" << std::endl;

	/*
		Test remaining fractions
		Frame size is (4*4096) * 8bit
		sizeof(TestStruct1) = 32 bit + 32 bit = 64 bit
		sizeof(Header) = 64 bit
		sizeof(Slot) = 32 bit
		64bit + x * 32 bit + x * 64 bit = 4 * 4096 * 8 bit
		<=> x = 1364,...
		We can store at most 1364 Teststructs in this test scenario on a given SlottedPage
	*/

	SlottedPage sut2;
	std::cout << "SlottedPageTest: Testing remaining space with 1364 TestStructs(2)" << std::endl;
	for(int i = 0; i < 1364; i++) {
		auto slotId = sut2.allocateSlot(sizeof(TestStruct2));
		assert(slotId == i);
		sut2.storeData(slotId, (char*)(new TestStruct2(i, i/2)));
	}

	assert(!sut2.canAllocateSlot(sizeof(TestStruct2)));

	auto stored = 1364 * sizeof(TestStruct2);
	auto remaining = SlottedPage::dataSize - stored - (1364 * 4); 
	std::cout << "SlottedPageTest: " << remaining << " bytes should be free" << std::endl;
	assert(sut2.getCurrentFreeSpace() == remaining);
	assert(sut2.getCompactedFreeSpace() == remaining);
	std::cout << "SlottedPageTest: " << remaining << " bytes are free" << std::endl;
	std::cout << "SlottedPageTest: Test successfull" << std::endl;
}