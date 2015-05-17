#include "pages/SlottedPage.hpp"
#include <assert.h>
#include <queue>

SlottedPage::SlottedPage() {
	header.numberSlots = 0;
	header.firstFreeSlot = 0;
	header.dataStart = SlottedPage::dataSize;
	header.freeFragments = 0;
}

SlottedPage::~SlottedPage() {

}

uint16_t SlottedPage::getCurrentFreeSpace() {
	auto lower = header.numberSlots * sizeof(Slot);
	auto upper = header.dataStart;

	// std::cout << "SlottedPage.getCurrentFreeSpace: lower=" << lower << std::endl;
	// std::cout << "SlottedPage.getCurrentFreeSpace: upper=" << upper << std::endl;
	return upper - lower;
}

uint16_t SlottedPage::getCompactedFreeSpace() {
	auto current = getCurrentFreeSpace();

	// std::cout << "SlottedPage.getCompactedFreeSpace: fragments=" << header.freeFragments << std::endl;
	return current + header.freeFragments;
}

bool SlottedPage::canAllocateSlot(uint16_t dataSize) {
	auto requiredSpace = dataSize + sizeof(Slot);

	if(getCompactedFreeSpace() >= requiredSpace) {

		// std::cout << "SlottedPage.canAllocateSlot: true" << std::endl;
		return true;
	} else {

		// std::cout << "SlottedPage.canAllocateSlot: false" << std::endl;
		return false;
	}
}


uint16_t SlottedPage::allocateSlot(uint16_t dataSize) {
	// std::cout << "SlottedPage.allocateSlot" << std::endl;

	assert(canAllocateSlot(dataSize));

	// compactify if needed
	// use priority queue to order slot lengths descending
	// @TODO: Would be nice to use a dynamically sized cache for performance reasons. (optional)
	if(getCurrentFreeSpace() < dataSize) {
		std::cout << "SlottedPage.allocateSlot: compactifying the data" << std::endl;

		// order the slots with a priority queue (length descending)
		std::priority_queue<Slot*, std::vector<Slot*>, SlotLengthGreater> q;
		for(int i = 0; i < header.numberSlots; i++) {
			if(!slots[i].isEmpty()) {
				q.push(slots + i);
			}
		}

		// allocate temporary data array and write the data in sorted order
		char cache[SlottedPage::dataSize];
		auto cacheDataStart = SlottedPage::dataSize;
		while(!q.empty()) {
			Slot *slotPtr = q.top();
			q.pop();
			cacheDataStart -= slotPtr -> length;
			memcpy(cache + cacheDataStart, data + slotPtr->offset, slotPtr -> length);

			// store the new offset in the slot.
			// (length stays of course the same)
			slotPtr -> offset = cacheDataStart;
		}

		// now store the compactified data in the page
		std::cout << "SlottedPage.allocateSlot: writing sorted data back" << std::endl;
		memcpy(data + cacheDataStart, cache + cacheDataStart, header.dataStart - cacheDataStart);
		header.freeFragments = 0;
		header.dataStart = cacheDataStart;
	}

	// use the firstFreeSlot as slotId
	uint16_t slotId = header.firstFreeSlot;
	// we checked that we can store the data already
	// just decrement the lower data pointer by dataSize
	header.dataStart -= dataSize;
	// save the new slot
	slots[slotId] = { .offset = header.dataStart, .length = dataSize };

	// std::cout << "SlottedPage.allocateSlot: slot=" << slotId << std::endl;
	// std::cout << "SlottedPage.allocateSlot: dataStart=" << header.dataStart << std::endl;
	// std::cout << "SlottedPage.allocateSlot: numberSlots=" << header.numberSlots << std::endl;

	updateFirstSlot();
	return slotId;
}


void SlottedPage::storeData(uint16_t slotId, char *newData) {
	auto first = slots[slotId].offset;
	auto length = slots[slotId].length;
	memcpy(data + first, newData, length);

	// std::cout << "SlottedPage.storeData: memcopy.first=" << first << std::endl;
	// std::cout << "SlottedPage.storeData: memcopy.length=" << length << std::endl;
}

void SlottedPage::updateFirstSlot() {
	auto newFirst = header.firstFreeSlot + 1;
	// first check if any of the slots is zero
	while(newFirst < header.numberSlots) {
		if (slots[newFirst].isEmpty()) {
			header.firstFreeSlot = newFirst;
			return;
		}
	}
	// when the while loop finishes newFirst points to the first byte after the original array
	// extend the array by one
	header.numberSlots++;
	header.firstFreeSlot = newFirst;

	// std::cout << "SlottedPage.updateFirstSlot: numberSlots=" << header.numberSlots << std::endl;
	// std::cout << "SlottedPage.updateFirstSlot: firstFreeSlot=" << header.firstFreeSlot << std::endl;
}