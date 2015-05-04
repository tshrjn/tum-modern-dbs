#ifndef BUFFERMANAGER_HPP
#define BUFFERMANAGER_HPP

#include <unordered_map>
#include <deque>
#include <mutex>

#include "BufferFrame.hpp"

class BufferManager
{
public:
    // Create new instance that keeps up to pageCount frames in main memory
    BufferManager(size_t pageCount);
    // Write all dirty frames to disk and free all resources
    ~BufferManager();

    /*
     * Retrieve a page given a pageID, indicate whether the page will be held exclusively or not.
     * It fails if no free frame is available and no frame can be freed.
     * The page ID splits into a segment ID and an actual page ID.
     * Each page is stored disk in a file with the same name as its segment ID.
     */
    BufferFrame& fixPage(uint64_t pageId, bool exclusive);

    /*
     * Return a frame to the buffer manager, indicating if it is dirty or not.
     * If dirty, the manager must write it back to disk. It doesn't have to write it back
     * immediately, but must write it back before unfixPage is called.
     */
    void unfixPage(BufferFrame& frame, bool isDirty);

private:
    // maximum number of bufferd pages
    size_t maxPageCount;

    // has table mutex
    std::mutex mtx;

    // Hashmap that maps page ids to the corresponding buffer frame
    std::unordered_map<uint64_t, BufferFrame*> bufferFrameMap;

    // Store file descriptors of opened segment files
    std::unordered_map<uint16_t, int> segmentsFdMap;

    // FIFO replacement strategy
	// We need to iterate over the queue in order to find the first unfixed frame
	// Therefore we better use a std::deqeue instead of a std::queue
    std::deque<BufferFrame*> fifo;

    // Check if we have space for another frame
    bool isFrameAvailable();

    // Get a file descriptor for the given segment ID
    int getSegmentFd(uint16_t segmentID);
};

#endif //  BUFFERMANAGER_HPP
