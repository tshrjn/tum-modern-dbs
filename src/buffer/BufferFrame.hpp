#ifndef BUFFERFRAME_HPP
#define BUFFERFRAME_HPP
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

const size_t blockSize = 4 * 4096;

enum FrameState
{
    empty, // data not loaded yet
    clean,  // data loaded, unmodified
    dirty	// data loaded, has changed
};

class BufferFrame
{
private:
    // File descriptor of the segment
    int segmentFd;
    uint64_t pageID;
    // (start) position of the page in the segment file
    off_t pageOffsetInFile;

    // current state of the page (empty (not loaded), clean, dirty)
    FrameState state;
    // Data contained in this page
    void* data;

    // Read page from disk
    void readPage();
    // Wrtie page to disk
    void writePage();

public:
    BufferFrame(int segmentFd, uint64_t pageID);
    ~BufferFrame();

    // Give access to the content of the buffered page
    void* getData();
    // Flush data to disk
    void flush();
};

#endif //  BUFFERFRAME_HPP
