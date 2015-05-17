#ifndef BUFFERFRAME_HPP
#define BUFFERFRAME_HPP
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <sys/types.h>
#include <pthread.h>
#include "buffer/PID.cpp"

enum FrameState
{
    empty, // data not loaded yet
    clean,  // data loaded, unmodified
    dirty	// data loaded, has changed
};

class BufferFrame
{
private:
    PID pageID;

    // file descriptor of segment
    int segmentFd;

    // (start) position of the page in the segment file
    off_t pageOffsetInFile;

    // Read/Write lock for a single bufferframe
    // This lock is used to implement the exclusive or shared access of the frame
    pthread_rwlock_t frameLock;

    // current state of the page (empty (not loaded), clean, dirty)
    FrameState state;
    // Data contained in this page
    void* data;

    // Read page from disk
    void readPage();
    // Write page to disk
    void writePage();

   

public:
    static const size_t frameSize = 4 * 4096;

    BufferFrame(PID pageID, int segmentFd);
    ~BufferFrame();

    // Give access to the content of the buffered page
    void* getData();
    // Flush data to disk
    void flush();

    void setDirty();

	PID getPageID() {
		return pageID;
	}

    // Exclusive and Non-Exclusive Locks
    bool lockWrite(bool blocking);
    bool lockRead(bool blocking);
    void unlock();
};

#endif //  BUFFERFRAME_HPP
