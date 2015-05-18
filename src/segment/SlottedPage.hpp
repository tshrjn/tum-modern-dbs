#ifndef H_SLOTTEDPAGE_HPP
#define H_SLOTTEDPAGE_HPP

#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cstdint>
#include "buffer/BufferFrame.hpp"
#include "segment/TID.cpp"
#include <iostream>

// Struct to hold a whole slottedPage
struct SlottedPage {

// typedefs
private:
	// Based on chapter 3 page 9
	// 64 bit for header
	typedef struct {
		// Number of currently used slots.
	    uint16_t numberSlots;

	    // To speed up the search for a free slot
	    uint16_t firstFreeSlot;

	    // Lower end of the data
	    // We use a union structure for slots && data therefore
	    // data[dataOffset] points to the first data byte
	    uint16_t dataOffset;

	    // Additional space that would be available after compactification
	    uint16_t fragmentedSpace;
	} Header;

	// A single slot
	// 32 bit for slot
	typedef struct {
		uint16_t offset;
		uint16_t length;

	public:
		bool isEmpty() {
			// The offset can never be dataSize as it is Out of bounds
			return offset == SlottedPage::dataSize;
		}

		void setEmpty() {
			offset = SlottedPage::dataSize;
			length = 0;
		}
	} Slot;

	// Single slot that stores the offset inside the single page and the length
	class SlotLengthGreater {
		public:
	    bool operator()(Slot* s1, Slot* s2) { // Returns true if s1 is smaller than s2
	    	return s1->length > s2->length;
	    }
	};

// Constants
public:	
	static const uint16_t pageSize = BufferFrame::frameSize;
	static const uint16_t headerSize = sizeof(Header);
	static const uint16_t slotSize = sizeof(Slot);
	static const uint16_t dataSize = pageSize - headerSize;

// Private fields
private:
	Header header;
	union {
		Slot slots[SlottedPage::dataSize / sizeof(Slot)];
		char data[SlottedPage::dataSize];
	};

	// compactify the data and remove empty fragments
	void compactify();


// Methods && Constructors
public:
	SlottedPage();
	~SlottedPage();

	// Return the space that is available without compactification
	uint16_t getCurrentFreeSpace();

	// Return the space that is available with compactification
	uint16_t getCompactedFreeSpace();

	// Check if the data can be stored in the page
	bool canAllocateSlot(uint16_t dataSize);

	// Check if the slot can be reallocated without a new TID reference on the same page
	bool canReallocateSlot(uint16_t slotId, uint16_t dataSize);

	// Store data in page and return the slot id
	// Assert (canAllocateSlot(data))
	uint16_t allocateSlot(uint16_t dataSize);

	// Shrink/Enlarge the slot on the same page (without TID redirect)
	// Assert (canReallocateSlot(slotId, dataSize))
	void reallocateSlot(uint16_t slotId, uint16_t dataSize);

	// Stores the data in the given slotId
	void storeData(uint16_t slotId, char *data);

	// Remove the slot
	void removeSlot(uint16_t slotId);

	// Find next free slot
	void updateFirstSlot();

	// Chapter 3 page 3
	// The lower 4 bits of the returned byte represent the logarithmic free space of the page
	// (not used at the moment)
	char getFreeSpaceNibble(bool upper);
};
#endif
