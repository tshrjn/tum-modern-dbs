#include "BufferManager.hpp"
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

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
 */
BufferFrame& BufferManager::fixPage(uint64_t pageID, bool exclusive)
{
    // @TODO: What if this or the existing entry should be exclusive?

    // Check if page is already in buffered, then just return existing frame
    auto entry = bufferFrameMap.find(pageID);
    if (entry != bufferFrameMap.end()) {
        return *entry->second;
    }

    // If the page isn't fixed yet, check if we have the space to load another frame.
    if (!isFrameAvailable() && !freeFrame(pageID)) {
        throw std::runtime_error("The buffer manager has no available frame left and is incapabable of freeing an existing one!");
    }

    std::cout << "pid: " << pageID << std::endl;
    int segmentFd = getSegmentFd(pageID >> 48);
    uint64_t actualPageID = pageID & ((1L << 48)-1);

    BufferFrame* frame = new BufferFrame(segmentFd, actualPageID);
    auto result = bufferFrameMap.insert(std::make_pair(pageID, frame));
    if (!result.second)
        throw std::runtime_error("Insert to buffer map not successful!");

    fifo.push(result.first->second);

    return *frame;
}

/*
 * Return a frame to the buffer manager, indicating if it is dirty or not.
 * If dirty, the manager must write it back to disk. It doesn't have to write it back
 * immediately, but must write it back before unfixPage is called.
 */
void BufferManager::unfixPage(BufferFrame& frame, bool isDirty)
{
    if (isDirty)
        frame.setDirty();
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
    std::cout << "CALLED FREEFRAME" << std::endl;
    BufferFrame* bufferFrame = fifo.front();
    bufferFrameMap.erase(pageID);
    fifo.pop();
    delete bufferFrame;

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
        std::cout << "return existing fd: " << segmentID << std::endl;
    } else {
        // Create a new segment file using the segmentID
        char filename[30];
        sprintf(filename, "data/%d", segmentID);
        // Open the segment file
        std::cout << "open segment file: " << segmentID << std::endl;
        fd = open(filename, O_RDWR | O_CREAT, 0600);
        if (fd < 0) {
            throw std::runtime_error(std::strerror(errno));
        }
        std::cout << "fd: " << fd << std::endl;
        // Store the file descriptor
        segmentsFdMap[segmentID] = fd;
    }
    return fd;
}