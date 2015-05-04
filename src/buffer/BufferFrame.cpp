#include "BufferFrame.hpp"
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

BufferFrame::BufferFrame(int segmentFd, uint64_t pageID)
    : segmentFd(segmentFd), pageID(pageID)
{
    // initialize the read write lock
    pthread_rwlock_init(&frameLock, NULL);

    // initially the data points to null and the state is empty
    data = nullptr;
    state = FrameState::empty;
    pageOffsetInFile = pageID * blockSize;
    std::cout << "page: " << pageID << "\t offset:" << pageOffsetInFile << std::endl;
}

BufferFrame::~BufferFrame()
{
    std::cout << "free data page " << pageID << std::endl;
    if (data != nullptr) {
        free(data);
        data = nullptr;
    }
}

// Give access to the content of the buffered page
void* BufferFrame::getData()
{
    std::cout << "get data from page " << pageID << std::endl;
    if (state == FrameState::empty)
        readPage();
    return data;
}

// Read page from disk
void BufferFrame::readPage()
{
    std::cout << "read data of page " << pageID << std::endl;
    data = malloc(blockSize);
    int bytesRead = pread(segmentFd, data, blockSize, pageOffsetInFile);

    state = FrameState::clean;
}

// Write page to disk
void BufferFrame::writePage()
{
    std::cout << "write data of page " << pageID << std::endl;
    int bytesRead = pwrite(segmentFd, data, blockSize, pageOffsetInFile);
    state = FrameState::clean;
}

// Flush frame to disk
void BufferFrame::flush()
{
    std::cout << "flush data of page " << pageID << std::endl;
    // Writeback only if page has been modified
    if (state == FrameState::dirty)
        writePage();
}

void BufferFrame::setDirty()
{
    state = FrameState::dirty;
}


// Lock frame for exclusive write
bool BufferFrame::lockWrite(bool blocking)
{ 
    if(!blocking) {
        return pthread_rwlock_trywrlock(&frameLock);
    } else {
        return pthread_rwlock_wrlock(&frameLock);
    }
}
// Lock frame for read
bool BufferFrame::lockRead(bool blocking)
{
    if(!blocking) {
        return pthread_rwlock_tryrdlock(&frameLock);
    } else {
        return pthread_rwlock_rdlock(&frameLock);
    }
}
// Release read or write
void BufferFrame::unlock()
{
    pthread_rwlock_unlock(&frameLock);
}