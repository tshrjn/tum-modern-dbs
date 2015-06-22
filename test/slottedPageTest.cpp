#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>
#include <unordered_map>

#include "buffer/BufferManager.hpp"
#include "segment/SlottedPage.hpp"
#include "segment/TID.cpp"

using namespace std;

struct TestStruct4 {
	uint16_t a;
	uint16_t b;
	TestStruct4(uint16_t v_a, uint16_t v_b) : a(v_a), b(v_b) {}
};

struct TestStruct8 {
	uint32_t a;
	uint32_t b;
	TestStruct8(uint32_t v_a, uint32_t v_b) : a(v_a), b(v_b) {}
};

struct TestStruct4096 {
	char dummyData[4096];
	TestStruct4096(char x) {
		for(int i = 0; i < 4096; i++) {
			dummyData[i] = x;
		}
	}
};

struct TestStruct6144 {
	char dummyData[6144];
	TestStruct6144(char x) {
		for(int i = 0; i < 6144; i++) {
			dummyData[i] = x;
		}
	}
};

int main(int argc, char* argv[]) {
	SlottedPage sut1;
	std::cout << "SlottedPageTest: Testing init" << std::endl;
	assert(sut1.getCurrentFreeSpace() == SlottedPage::dataSize);
	assert(sut1.getCompactedFreeSpace() == SlottedPage::dataSize);

	/*
		Test perfect fitting
		Frame size is (4*4096) * 8bit
		sizeof(TestStruct4) = 2*16 bit = 32 bit
		sizeof(Header) = 64 bit
		sizeof(Slot) = 32 bit
		64bit + x * 32 bit + x * 32 bit = 4 * 4096 * 8 bit
		<=> x = 2047
		We can store exactly 2047 Teststructs in this test scenario on a given SlottedPage
	*/

	std::cout << "SlottedPageTest: Testing perfect fit with 2047 TestStructs (4 Byte)" << std::endl;
	for(int i = 0; i < 2047; i++) {
		auto slotId = sut1.allocateSlot(sizeof(TestStruct4));
		assert(slotId == i);
		sut1.storeData(slotId, (char*)(new TestStruct4(i, i/2)));
	}

	// Test subsequent retrieval
	std::cout << "SlottedPageTest: Test subsequent read" << std::endl;
	for(int i = 0; i < 2047; i++) {
		char* data = sut1.getData(i);
		TestStruct4* testStruct = (TestStruct4*) data;
		assert(testStruct->a == i);
		assert(testStruct->b == i/2);
	}

	// Check if the page is full now
	assert(!sut1.canAllocateSlot(sizeof(TestStruct4)));
	assert(sut1.getCurrentFreeSpace() == 0);
	assert(sut1.getCompactedFreeSpace() == 0);
	std::cout << "SlottedPageTest: Test successfull" << std::endl;

	/*
		Test remaining fractions
		Frame size is (4*4096) * 8bit
		sizeof(TestStruct4) = 32 bit + 32 bit = 64 bit
		sizeof(Header) = 64 bit
		sizeof(Slot) = 32 bit
		64bit + x * 32 bit + x * 64 bit = 4 * 4096 * 8 bit
		<=> x = 1364,...
		We can store at most 1364 Teststructs in this test scenario on a given SlottedPage
	*/

	SlottedPage sut2;
	std::cout << "SlottedPageTest: Testing remaining space with 1364 TestStructs (8 Byte)" << std::endl;
	for(int i = 0; i < 1364; i++) {
		auto slotId = sut2.allocateSlot(sizeof(TestStruct8));
		assert(slotId == i);
		sut2.storeData(slotId, (char*)(new TestStruct8(i, i/2)));
	}

	// Test subsequent retrieval
	std::cout << "SlottedPageTest: Test subsequent read" << std::endl;
	for(int i = 0; i < 1364; i++) {
		char* data = sut2.getData(i);
		TestStruct8* testStruct = (TestStruct8*) data;
		assert(testStruct->a == i);
		assert(testStruct->b == i/2);
	}

	assert(!sut2.canAllocateSlot(sizeof(TestStruct8)));
	auto stored = 1364 * sizeof(TestStruct8);
	auto remaining = SlottedPage::dataSize - stored - (1364 * 4); 
	assert(sut2.getCurrentFreeSpace() == remaining);
	assert(sut2.getCompactedFreeSpace() == remaining);
	std::cout << "SlottedPageTest: Test successfull" << std::endl;


	/*
		Test expansion with compactify
	*/
	SlottedPage sut3;
	std::cout << "SlottedPageTest: Testing expansion with compactify" << std::endl;
	assert(sut3.canAllocateSlot(sizeof(TestStruct4096)));
	assert(sut3.canAllocateSlot(sizeof(TestStruct4096)));
	auto sut3_1 = sut3.allocateSlot(sizeof(TestStruct4096));
	sut3.storeData(sut3_1, (char*)(new TestStruct4096((char) 1)));
	auto sut3_2 = sut3.allocateSlot(sizeof(TestStruct4096));
	sut3.storeData(sut3_2, (char*)(new TestStruct4096((char) 2)));

	// 2 * 4096 - 2 slots - 1 header - 1 new slot
	auto used = sizeof(TestStruct4096) * 2 + SlottedPage::slotSize * 2;
	auto free = SlottedPage::dataSize - used;
	assert(sut3.getCurrentFreeSpace() == free);
	assert(sut3.canReallocateSlot(sut3_1, sizeof(TestStruct6144)));
	auto sut3_3 = sut3.allocateSlot(sizeof(TestStruct4096));
	sut3.storeData(sut3_3, (char*)(new TestStruct4096((char) 3)));

	// reallocate
	assert(sut3.canReallocateSlot(sut3_1, sizeof(TestStruct6144)));
	sut3.reallocateSlot(sut3_1, sizeof(TestStruct6144));
	sut3.storeData(sut3_1, (char*)(new TestStruct6144((char) 1)));

	// check if data is corrent
	char* sut3_1_data_1 = (char*)sut3.getData(sut3_1);
	char* sut3_2_data_1 = (char*)sut3.getData(sut3_2);
	char* sut3_3_data_1 = (char*)sut3.getData(sut3_3);
	for(int i=0; i<6144; i++) {
		assert(sut3_1_data_1[i] == (char)1);
	}
	for(int i=0; i<4096; i++) {
		assert(sut3_2_data_1[i] == (char)2);
		assert(sut3_3_data_1[i] == (char)3);
	}
	std::cout << "SlottedPageTest: Read successfull" << std::endl;

	// check free space
	used = sizeof(TestStruct4096) * 2 + sizeof(TestStruct6144)+ SlottedPage::slotSize * 3;
	free = SlottedPage::dataSize - used;
	assert(sut3.getCurrentFreeSpace() == free);
	std::cout << "SlottedPageTest: Test successfull" << std::endl;

	/*
		Test fragmentation
	*/
	std::cout << "SlottedPageTest: Testing fragmentation" << std::endl;
	sut3.removeSlot(sut3_2);
	assert(sut3.getCurrentFreeSpace() == free);
	assert(sut3.getCompactedFreeSpace() == free + sizeof(TestStruct4096));
	std::cout << "SlottedPageTest: Test successfull" << std::endl;

	/*
		Test allocate with compacitfy
	*/
	std::cout << "SlottedPageTest: Testing allocation with compactify" << std::endl;
	assert(sut3.canAllocateSlot(sizeof(TestStruct4096)));
	auto sut3_4 = sut3.allocateSlot(sizeof(TestStruct4096));
	sut3.storeData(sut3_4, (char*)(new TestStruct4096((char) 4)));

	// check if data is corrent
	char* sut3_1_data_2 = (char*)sut3.getData(sut3_1);
	char* sut3_3_data_2 = (char*)sut3.getData(sut3_3);
	char* sut3_4_data_2 = (char*)sut3.getData(sut3_4);
	for(int i=0; i<6144; i++) {
		assert(sut3_1_data_2[i] == (char)1);
	}
	for(int i=0; i<4096; i++) {
		assert(sut3_3_data_2[i] == (char)3);
		assert(sut3_4_data_2[i] == (char)4);
	}
	std::cout << "SlottedPageTest: Read successfull" << std::endl;

	assert(sut3.getCurrentFreeSpace() == free - SlottedPage::slotSize);
	assert(sut3.getCompactedFreeSpace() == free - SlottedPage::slotSize);
	std::cout << "SlottedPageTest: Test successfull" << std::endl;
}