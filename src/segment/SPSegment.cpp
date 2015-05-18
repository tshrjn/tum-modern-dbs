#include "segment/SPSegment.hpp"


TID SPSegment::insert(const Record& r) {
	auto recordSize = r.getTotalLen();
	std::cout << "SPSegment.insert: recordSize=" << recordSize << std::endl;

	for(int i = 0; i < numberPages; i++) {
		// first just check if there is enough space
		// @TODO: exclusive lock on every page lookup might be super slow
		auto frame = bufferManager.fixPage(PID(segmentId,i), true);
		SlottedPage* page = static_cast<SlottedPage*>(frame->getData());

		if(page->canAllocateSlot(recordSize)) {
			auto slotId = page->allocateSlot(recordSize);
			// std::cout << "allocated slot " << slotId << " with length " << page->getLength(slotId) << std::endl;
			page->storeData(slotId, (char *) &r);
			// std::cout << "stored " << recordSize << std::endl;
			
			bufferManager.unfixPage(frame, true);
			// std::cout << "unfixed" << std::endl;

			std::cout << "SPSegment.insert: Returing TID (" << segmentId << "," << slotId << ")" << std::endl;
			return TID(i, slotId);

		} else {
			bufferManager.unfixPage(frame, false);
		}
	}

	// we need to create a new slottedPage
	auto frame = bufferManager.fixPage(PID(segmentId,numberPages++), true);
	auto dataPtr = frame->getData();
	memcpy(dataPtr, new SlottedPage(), BufferFrame::frameSize);
	SlottedPage* page = static_cast<SlottedPage*>(dataPtr);

	// allocate a slot and store the record
	auto slotId = page->allocateSlot(recordSize);
	page->storeData(slotId, (char *) &r);
	bufferManager.unfixPage(frame, true);
	
	std::cout << "SPSegment.insert: Returing TID (" << segmentId << "," << slotId << ") on new Page" << std::endl;
	return TID(segmentId,slotId);
}

bool SPSegment::remove(TID tid) {
	std::cout << "SPSegment.remove" << std::endl;

	auto pageId = tid.getPage();
	auto slotId = tid.getSlot();
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), true);
	SlottedPage* page = static_cast<SlottedPage*>(frame->getData());
	page->removeSlot(slotId);
	bufferManager.unfixPage(frame, true);
	return true;
}

Record& SPSegment::lookup(TID tid) {
	std::cout << "SPSegment.lookup" << std::endl;

	auto pageId = tid.getPage();
	auto slotId = tid.getSlot();
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), false);
	SlottedPage* page = (SlottedPage*) frame->getData();
	Record* record = (Record*) page->getData(slotId);
	return *record;
}

bool SPSegment::update(TID tid, const Record& r) {
	std::cout << "SPSegment.update" << std::endl;

	auto pageId = tid.getPage();
	auto slotId = tid.getSlot();
	auto frame = bufferManager.fixPage(PID(segmentId, pageId), true);
	SlottedPage* page = (SlottedPage*) frame->getData();
	Record* oldRecordPtr = (Record*) page->getData(slotId);

	// first get old and check if the size is the same
	Record& oldRecord =  *oldRecordPtr;
	if(oldRecord.getLen() == r.getLen()){
		// just overwrite
		page->storeData(slotId, (char *) &r);

	} else {
		auto newSize = r.getLen() + sizeof(unsigned);

		if(!page->canReallocateSlot(slotId, newSize)) {
			bufferManager.unfixPage(frame, false);
			return false;
		}

		// reallocate
		page->reallocateSlot(slotId, newSize);
		page->storeData(slotId, (char *)&r);
	}

	bufferManager.unfixPage(frame, true);
	return true;
}
