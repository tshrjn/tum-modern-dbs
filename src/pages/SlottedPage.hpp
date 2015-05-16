#ifndef H_SLOTTEDPAGE_HPP
#define H_SLOTTEDPAGE_HPP

#include <cstdio>
#include <cstdint>

#include <buffer/BufferFrame.hpp>

// Single slot that stores the offset inside the single page and the length
struct Slot {
	uint32_t offset;
	uint32_t length;
};

// Based on chapter 3 page 9
struct Header {
		// Number of currently used slots.
	    uint32_t slotCount;

	    // To speed up the search for a free slot
	    uint32_t firstFreeSlot;

	    // Lower end of the data
	    // We use a union structure for slots && data therefore
	    // data[dataStart] points to the first data byte
	    size_t dataStart;

	    // Space that would be available after compactification
	    size_t freeSpace;
};

// Struct to hold a whole slottedPage
struct SlottedPage {
	static const size_t pageSize = BufferFrame::frameSize;
	static const size_t headerSize = sizeof(Header);

public:
	Header header;

	union {
		Slot slots[(pageSize - headerSize) / sizeof(Slot)];
		char data[(pageSize - headerSize) / sizeof(Slot)];
	};
};


#endif
