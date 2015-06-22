#include "segment/SPSegment.hpp"


TID SPSegment::insert(Record* r) {
	auto buffer = r->serialize();
	auto bufferSize = r->serializedSize();
	// BufferFrame::printHex(buffer, r->serializedSize());

	for(int i = 0; i < numberPages; i++) {
		// first just check if there is enough space
		// @TODO: exclusive lock on every page lookup might be super slow
		auto frame = bufferManager.fixPage(PID(segmentId,i), true);
		SlottedPage* page = static_cast<SlottedPage*>(frame->getData());

		if(page->canAllocateSlot(bufferSize)) {
			auto slotId = page->allocateSlot(bufferSize);
			// std::cout << "allocated slot " << slotId << " with length " << page->getLength(slotId) << std::endl;
			page->storeData(slotId, buffer);
			// std::cout << "stored " << bufferSize << std::endl;
			
			bufferManager.unfixPage(frame, true);
			// std::cout << "unfixed" << std::endl;

			std::cout << "SPSegment.insert: Returing TID (" << i << "," << slotId << ")" << std::endl;
			return TID(i, slotId);

		} else {
			bufferManager.unfixPage(frame, false);
		}
	}

	// we need to create a new slottedPage
	auto pageId = numberPages++;
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), true);
	auto dataPtr = frame->getData();
	memcpy(dataPtr, new SlottedPage(), BufferFrame::frameSize);
	SlottedPage* page = static_cast<SlottedPage*>(dataPtr);

	// allocate a slot and store the record
	auto slotId = page->allocateSlot(bufferSize);
	page->storeData(slotId, buffer);
	bufferManager.unfixPage(frame, true);
	
	std::cout << "SPSegment.insert: Returing TID (" << pageId << "," << slotId << ") on new Page" << std::endl;
	return TID(pageId,slotId);
}

bool SPSegment::remove(TID tid) {
	std::cout << "SPSegment.remove of TID " << (std::string) tid << std::endl;

	auto pageId = tid.getPage();
	auto slotId = tid.getSlot();
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), true);
	SlottedPage* page = static_cast<SlottedPage*>(frame->getData());
	page->removeSlot(slotId);
	bufferManager.unfixPage(frame, true);
	return true;
}

Record* SPSegment::lookup(TID tid) {
	std::cout << "SPSegment.lookup for TID " << (std::string) tid << std::endl;

	auto pageId = tid.getPage();
	auto slotId = tid.getSlot();
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), false);
	SlottedPage* page = (SlottedPage*) frame->getData();

	auto serialized = page->getData(slotId);
	auto result = Record::deserialize(serialized);
	
	bufferManager.unfixPage(frame, false);
	return result;
}

bool SPSegment::update(TID tid, Record* r) {
	std::cout << "SPSegment.update TID " << (std::string) tid << std::endl;

	auto buffer = r->serialize();
	auto bufferSize = r->serializedSize();

	// first get old and check if the size is the same
	auto pageId = tid.getPage();
	auto slotId = tid.getSlot();
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), true);
	SlottedPage* page = (SlottedPage*) frame->getData();
	auto serialized = page->getData(slotId);
	auto oldRecord = Record::deserialize(serialized);

	if(oldRecord->getLen() == r->getLen()){
		// just overwrite
		page->storeData(slotId, buffer);

	} else {
		if(!page->canReallocateSlot(slotId, bufferSize)) {
			std::cout << "SPSegment.update could not allocate size " << bufferSize << " (from " << oldRecord->getLen() << ")" << std::endl;

			bufferManager.unfixPage(frame, false);
			return false;
		}

		// reallocate
		page->reallocateSlot(slotId, bufferSize);
		page->storeData(slotId, buffer);
	}

	bufferManager.unfixPage(frame, true);
	return true;
}
