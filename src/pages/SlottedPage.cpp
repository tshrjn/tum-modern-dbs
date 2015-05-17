#include "pages/SlottedPage.hpp"
#include <assert.h>
#include <queue>

SlottedPage::SlottedPage() {
	header.numberSlots = 0;
	header.firstFreeSlot = 0;
	header.dataOffset = SlottedPage::dataSize;
	header.freeFragments = 0;
}

SlottedPage::~SlottedPage() {

}

uint16_t SlottedPage::getCurrentFreeSpace() {
	auto lower = header.numberSlots * sizeof(Slot);
	auto upper = header.dataOffset;
	return upper - lower;
}

uint16_t SlottedPage::getCompactedFreeSpace() {
	auto current = getCurrentFreeSpace();
	return current + header.freeFragments;
}

bool SlottedPage::canAllocateSlot(uint16_t dataSize) {
	auto requiredSpace = dataSize + sizeof(Slot);
	return getCompactedFreeSpace() >= requiredSpace;
}

bool SlottedPage::canReallocateSlot(uint16_t slotId, uint16_t dataSize) {
	auto requiredSpace = dataSize;
	return getCompactedFreeSpace() - slots[slotId].length >= requiredSpace;
}

void SlottedPage::compactify() {
	std::cout << "SlottedPage.allocateSlot: compactifying the data" << std::endl;
	// order the slots with a priority queue (length descending)
	std::priority_queue<Slot*, std::vector<Slot*>, SlotLengthGreater> q;
	for(int i = 0; i < header.numberSlots; i++) {
		if(!slots[i].isEmpty()) {
			q.push(slots + i);
		}
	}

	// allocate temporary data array and write the data in sorted order
	// @TODO: Would be nice to use a dynamically sized cache for performance reasons. (optional)
	char cache[SlottedPage::dataSize];
	auto cacheDataOffset = SlottedPage::dataSize;
	while(!q.empty()) {
		Slot *slotPtr = q.top();
		q.pop();
		cacheDataOffset -= slotPtr -> length;
		memcpy(cache + cacheDataOffset, data + slotPtr->offset, slotPtr -> length);

		// store the new offset in the slot.
		// (length stays of course the same)
		slotPtr -> offset = cacheDataOffset;
	}

	// now store the compactified data in the page
	std::cout << "SlottedPage.allocateSlot: writing sorted data back" << std::endl;
	memcpy(data + cacheDataOffset, cache + cacheDataOffset, header.dataOffset - cacheDataOffset);
	header.freeFragments = 0;
	header.dataOffset = cacheDataOffset;
}

uint16_t SlottedPage::allocateSlot(uint16_t dataSize) {
	assert(canAllocateSlot(dataSize));

	// compactify if needed
	if(getCurrentFreeSpace() < dataSize) {
		compactify();
	}

	// use the firstFreeSlot as slotId
	uint16_t slotId = header.firstFreeSlot;

	// we checked that we can store the data already
	// just decrement the lower data pointer by dataSize
	header.dataOffset -= dataSize;

	// save the new slot
	slots[slotId] = { .offset = header.dataOffset, .length = dataSize };

	updateFirstSlot();
	return slotId;
}

void SlottedPage::removeSlot(uint16_t slotId) {
	slots[slotId] = { .offset = SlottedPage::dataSize, .length = 0 };
}

void SlottedPage::reallocateSlot(uint16_t slotId, uint16_t newDataSize) {
	assert(canReallocateSlot(slotId, newDataSize));

	// Attention: No need to MOVE the data as pointers will be invalidated anyway

	// Compactify if the data cant be appended in front
	if(getCurrentFreeSpace() < newDataSize) {
		removeSlot(slotId);
		compactify();
	}

	// @TODO: in case of shrinking we could work in place 
	// With this implementation we just append the new data in the front
	// probably valid assumption: data gets bigger most of the time
	header.dataOffset -= newDataSize;

	// Rewrite the Slot
	slots[slotId] = { .offset = header.dataOffset, .length = newDataSize };
}


void SlottedPage::storeData(uint16_t slotId, char *newData) {
	auto first = slots[slotId].offset;
	auto length = slots[slotId].length;
	memcpy(data + first, newData, length);
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
}