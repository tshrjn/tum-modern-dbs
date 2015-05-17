#ifndef SEGMENT_HPP
#define SEGMENT_HPP
#include "buffer/BufferManager.hpp"

class Segment {
protected:
    /**
     * Segment id
     */
    uint64_t id;

    /**
     * Size in number of pages.
     */
    size_t size;

    /**
     * Reference to global buffer manager instance.
     */
    BufferManager& bufferManager;

public:
    Segment(BufferManager &bufferManager, uint64_t id) : id(id), size(0), bufferManager(bufferManager) {}

    uint64_t getID() {
        return id;
    }

    size_t getSize() {
        return size;
    }
};


#endif //SEGMENT_HPP
