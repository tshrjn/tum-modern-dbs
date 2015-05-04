#include "BufferManager.hpp"
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
BufferFrame& BufferManager::fixPage(uint64_t pageID, bool exclusive) {
    BufferFrame *foundFrame = nullptr;

    // Lock the buffer manager
    // we need the lock as every page could be freed spontaneously
    mtx.lock();

    // Check if page is already in buffer, then just return existing frame
    auto entry = bufferFrameMap.find(pageID);
    if (entry == bufferFrameMap.end()) {
        foundFrame = entry->second;

        // First try to lock the frame without block
        bool locked = false;
        if (exclusive) {
            locked = foundFrame->lockWrite(false);
        } else {
            locked = foundFrame->lockRead(false);
        }

        // Unlock the BufferManager
        mtx.unlock();

        // If we could not lock it directly we wait for the unlock
        if(!locked) {
            if (exclusive) {
                locked = foundFrame->lockWrite(true);
            } else {
                locked = foundFrame->lockRead(true);
            }
        }
    } else {
        // Frame could not be found
        // Try to find a free slot || replace dirty pages

        // Try to replace something
        if (!isFrameAvailable()) {
            // ...
        }

        // Create new frame
        std::cout << "pid: " << pageID << std::endl;
        int segmentFd = getSegmentFd(pageID >> 48);
        uint64_t actualPageID = pageID & ((1L << 48)-1);
        BufferFrame* frame = new BufferFrame(segmentFd, actualPageID);
        auto result = bufferFrameMap.insert(std::make_pair(pageID, frame));
        if (!result.second) {
            throw std::runtime_error("Insert to buffer map not successful!");
        }
        fifo.push(result.first->second);

        // Did not work
        // Either the FrameMap does not contain empty slots
        // Or there are no Frames that can be replaced
        if (foundFrame == nullptr) {
            mtx.unlock();
            throw std::runtime_error("Could not replace an existing frame!");
        }

        // Frame is fresh in buffer -> Lock immediately
        if (exclusive) {
            foundFrame->lockWrite(true);
        } else {
            foundFrame->lockRead(true);
        }

        // Unlock BufferManager
        mtx.unlock();
    }
    return *foundFrame;
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
    frame.unlock();
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
