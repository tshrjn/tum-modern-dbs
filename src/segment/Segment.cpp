#ifndef SEGMENT_HPP
#define SEGMENT_HPP
#include "buffer/BufferManager.hpp"
#include <atomic>

class Segment {
protected:
    /**
     * Segment id
     */
    uint16_t segmentId;

    /**
     * Size in number of pages.
     */
    std::atomic<size_t> size;

    /**
     * Reference to global buffer manager instance.
     */
    BufferManager& bufferManager;

public:
    Segment(BufferManager &bufferManager, uint16_t segmentId) : segmentId(segmentId), size(0), bufferManager(bufferManager) {}

    uint16_t getSegmentId() {
        return segmentId;
    }

    size_t getSize() {
        return size.load();
    }
};


#endif //SEGMENT_HPP
