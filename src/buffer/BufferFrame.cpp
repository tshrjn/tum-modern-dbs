#include "BufferFrame.hpp"
#include <unistd.h>

BufferFrame::BufferFrame(uint16_t segmentFd, uint64_t pageID)
    : segmentFd(segmentFd), pageID(pageID)
{
    state = FrameState::empty;
    pageOffsetInFile = pageID * blockSize;
}

BufferFrame::~BufferFrame()
{
    free(data);
}

// Give access to the content of the buffered page
void* BufferFrame::getData()
{
    if (state == FrameState::empty)
        readPage();
    return data;
}

// Read page from disk
void BufferFrame::readPage()
{
    data = malloc(blockSize);
    pread(segmentFd, data, blockSize, pageOffsetInFile);
    state = FrameState::clean;
}

// Write page to disk
void BufferFrame::writePage() {
    pwrite(segmentFd, data, blockSize, pageOffsetInFile);
    state = FrameState::clean;
}

// Flush frame to disk
void BufferFrame::flush() {
    // Writeback only if page has been modified
    if (state == FrameState::dirty)
        writePage();
}