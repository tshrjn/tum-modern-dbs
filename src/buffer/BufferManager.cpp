#include "BufferManager.hpp"
#include <exception>
#include <fcntl.h>
#include <unistd.h>

//@TODO: Multi-threading support

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
        kv.second.flush();
    // close all opened file descriptors
    for (auto& kv: segmentsFdMap)
        close(kv.second);
}

/*
 * Retrieve a page given a pageID, indicate whether the page will be held exclusively or not.
 * It fails if no free frame is available and no frame can be freed.
 * The page ID splits into a segment ID and an actual page ID.
 * Each page is stored disk in a file with the same name as its segment ID.
 */
BufferFrame& BufferManager::fixPage(uint64_t pageID, bool exclusive)
{
    // @TODO: What if this or the existing entry should be exclusive?

    // Check if page is already in buffered, then just return existing frame
    auto entry = bufferFrameMap.find(pageID);
    if (entry != bufferFrameMap.end()) {
        return entry->second;
    }

    // If the page isn't fixed yet, check if we have the space to load another frame.
    if (!isFrameAvailable() && !freeFrame(pageID)) {
        throw std::runtime_error("The buffer manager has no available frame left and is incapabable of freeing an existing one!");
    }

    uint16_t segmentFd = getSegmentFd(pageID >> 48);
    uint64_t actualPageID = pageID & ((1L << 48)-1);

    auto result = bufferFrameMap.emplace(std::piecewise_construct, std::forward_as_tuple(pageID), std::forward_as_tuple(segmentFd, actualPageID));
    if (!result.second) {
        throw std::runtime_error("Insert to buffer map not successful!");
    }
    fifo.push(result.first->second);

    return result.first->second;
}

/*
 * Return a frame to the buffer manager, indicating if it is dirty or not.
 * If dirty, the manager must write it back to disk. It doesn't have to write it back
 * immediately, but must write it back before unfixPage is called.
 */
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
    if (isDirty)
        frame.flush();
}

/*
 * Returns true if frame is available, otherwise false.
 */
bool BufferManager::isFrameAvailable() {
    return bufferFrameMap.size() < maxPageCount;
}

/*
 * Attempts to remove a page from main memory.
 * Returns true if successful, false otherwise.
 */
bool BufferManager::freeFrame(uint64_t pageID) {
    // @TODO: When can't we just write a page back to disk?
    BufferFrame bufferFrame = fifo.front();
    bufferFrameMap.erase(pageID);
    fifo.pop();

    return true;
}

/**
 * The name of the segment file is segmentID and is stored in the subdirectory data/.
 */
int BufferManager::getSegmentFd(uint16_t segmentID) {
    int fd;

    // Check if we already have the file open
    auto entry = segmentsFdMap.find(segmentID);
    if (entry != segmentsFdMap.end()) {
        fd = entry->second;
    } else {
        // Create a new segment file using the segmentID
        char filename[15];
        sprintf(filename, "data/%d", segmentID);
        // Open the segment file
        fd = open(filename, O_RDWR | O_CREAT, 0660);
        if (fd < 0) {
            throw std::runtime_error("Something went wrong when trying to open the segment file!");
        }
        // Store the file descriptor
        segmentsFdMap[segmentID] = fd;
    }
    return fd;
}