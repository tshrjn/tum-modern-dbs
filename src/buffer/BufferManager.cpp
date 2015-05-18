#include "buffer/BufferManager.hpp"
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

// Create new instance that keeps up to pageCount frames in main memory
BufferManager::BufferManager(size_t pageCount)
        : maxPageCount(pageCount)
{

    bufferFrameMap.reserve(maxPageCount);
}

// Write all dirty frames to disk and free all resources
BufferManager::~BufferManager()
{
    // write dirty pages back to file
    for (auto& kv: bufferFrameMap)
        kv.second->flush();
    // close all opened file descriptors
    for (auto& kv: segmentsFdMap)
        close(kv.second);

}

/*
 * Retrieve a page given a pageID, indicate whether the page will be held exclusively or not.
 * It fails if no free frame is available and no frame can be freed.
 * The page ID splits into a segment ID and an actual page ID.
 * Each page is stored disk in a file with the same name as its segment ID.
 *
 * If the page is accessed exclusively the write lock is acquired and all readers are blocked
 * If the page is not accessed exclusively the read lock is acquired
 */
BufferFrame& BufferManager::fixPage(PID pageID, bool exclusive) {
	std::cout << "BufferManager.fix" << std::endl;

    BufferFrame *frame = nullptr;

    // Lock the buffer manager
    // we need the lock as every page could be freed spontaneously
    mtx.lock();

    // Check if page is already in buffer, then just return existing frame (with rwlock)
    auto entry = bufferFrameMap.find(pageID);
    if (entry != bufferFrameMap.end()) {
		std::cout << "BufferManager.fix: Requested PageID " << (std::string) pageID << " is already in Buffer" << std::endl;

        frame = entry->second;

        // First try to lock the frame without block
        bool locked = false;
        if (exclusive) {
            locked = frame->lockWrite(false);
        } else {
            locked = frame->lockRead(false);
        }

         // Unlock the BufferManager inside lock
        // @TODO: do this earlier - race conditions! 
        mtx.unlock();

        // If we could not lock it directly we wait for the unlock
        if (!locked) {
            if (exclusive) {
				std::cout << "BufferManager.fix: Needed to unlock buffer and wait for exclusive lock" \
				   	<< std::endl;
                locked = frame->lockWrite(true);
            } else {
				std::cout << "BufferManager.fix: Needed to unlock buffer and wait for non-exclusive lock" \
					<< std::endl;
                locked = frame->lockRead(true);
            }
        }
    } else {
		std::cout << "BufferManager.fix: Requested PageID " << (std::string)  pageID \
			<< " was not in Buffer" << std::endl;

        // Frame could not be found
        // Try to find a free slot || replace unfixed pages
        if (!isFrameAvailable()) {
			std::cout << "BufferManager.fix: Buffer does not contain free frames" << std::endl;

            // Search a page that is not locked (framestate is not important)
			BufferFrame *replacementCandidate = nullptr;
			for (std::deque<BufferFrame*>::iterator it = fifo.begin();
					it!=fifo.end(); ++it) {	
				// Just try to acquire an exclusive lock without block
				if((*it)->lockWrite(false)) {
					replacementCandidate = *it;
					fifo.erase(it);
					std::cout << "BufferManager.fix: Found replacement candidate with pageID " \
						<< (std::string) replacementCandidate->getPageID() << std::endl;
					break;
				}
			}
			if (replacementCandidate == nullptr) {
				throw std::runtime_error("BufferManager.fix: Could not allocate space for new buffer frame");
			} else {

				// Write page to disk if dirty
				replacementCandidate->flush();
				bufferFrameMap.erase(replacementCandidate->getPageID());
				delete replacementCandidate;
			}
        } else {
			std::cout << "BufferManager.fix: Buffer contains free frames" << std::endl;
		}

        // Create new frame
        std::cout << "BufferManager.fix: Creating new Frame with pageID: " << (std::string) pageID << std::endl;
        int segmentFd = getSegmentFd(pageID.getSegment());
        frame = new BufferFrame(pageID, segmentFd);
        auto result = bufferFrameMap.insert(std::make_pair(pageID, frame));
        if (!result.second) {
            throw std::runtime_error("BufferManager.fix: Insert to buffer map not successful!");
        }
        fifo.push_back(result.first->second);

        // Frame is fresh in buffer -> Lock immediately
        if (exclusive) {
            frame->lockWrite(true);
        } else {
            frame->lockRead(true);
        }

        // Unlock BufferManager
        // @TODO: do this earlier - race conditions! 
        mtx.unlock();
    }
    return *frame;
}

/*
 * Return a frame to the buffer manager, indicating if it is dirty or not.
 * If dirty, the manager must write it back to disk. It doesn't have to write it back
 * immediately, but must write it back before unfixPage is called.
 */
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
	std::cout << "BufferManager.unfix" << std::endl;
    if (isDirty) {
        frame.setDirty();
	}
    frame.unlock();
	std::cout << "BufferManager.unfix: Unlocked page " << (std::string) frame.getPageID() << std::endl;
}

/*
 * Returns true if frame is available, otherwise false.
 */
bool BufferManager::isFrameAvailable() {
    return bufferFrameMap.size() < maxPageCount;
}


/**
 * The name of the segment file is segmentID and is stored in the subdirectory data/.
 */
int BufferManager::getSegmentFd(uint16_t segmentID) {
	std::cout << "BufferManager.getSegment" << std::endl;
	
	int fd;

    // Check if we already have the file open
    auto entry = segmentsFdMap.find(segmentID);
    if (entry != segmentsFdMap.end()) {
        fd = entry->second;
        std::cout << "BufferManager.getSegment: return existing fd=" << segmentID << std::endl;
    } else {
        // Create a new segment file using the segmentID
        char filename[30];
        sprintf(filename, "data/%d", segmentID);
        // Open the segment file
        std::cout << "BufferManager.getSegment: open segment file " << segmentID << std::endl;
        fd = open(filename, O_RDWR | O_CREAT, 0600);
        if (fd < 0) {
			std::cout << "BufferManager.getSegment: failed to open file" << std::endl;
            throw std::runtime_error(std::strerror(errno));
        }
        std::cout << "BufferManager.getSegment: new file descriptor " << fd << std::endl;
        // Store the file descriptor
        segmentsFdMap[segmentID] = fd;
    }
    return fd;
}

void BufferManager::flushAll() {
    for (auto& kv: bufferFrameMap) {
        kv.second->flush();
    }
}