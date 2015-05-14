#ifndef SPSEGMENT_HPP
#define SPSEGMENT_HPP

#include "Record.hpp"
#include "TID.hpp"
#include "Segment.hpp"

class SPSegment : Segment {
public:
    struct Header {

        /*
         * Number of currently used slots.
         */
        uint32_t slotCount;

        /*
         * Current offset of the first data block from the beginning of the page.
         */
        off_t dataOffset;

        /*
         * Fragmented space that can potentially be used after compaction.
         */
        size_t fragmentedSpace;


        Header() : slotCount(0), dataOffset(blockSize), fragmentedSpace(blockSize-sizeof(Header)) {}
    };

    struct Slot {
        /*
         * Offset of the start of the data item from the beginning of the segment.
         */
        uint16_t offset;

        /*
         * Length of the data item.
         */
        uint16_t length;

        Slot() : offset(0), length(0) {}
    };

    /*
     * Constructor is given a reference to the buffer manager and a segment id.
     */
    SPSegment(BufferManager& bm, uint64_t id) : Segment(bufferManager, id) {};

    /*
     * Searches through the segment's pages looking for a page with enough space to store r.
     * Returns the TID identifying the location where r was stored.
     *
     * Note: This can be implemented much more efficiently with a free space bitmap
     * as described in chapter 3, slide 3, but you are not required to do this.
     */
    TID SPSegment::insert(const Record& r);

    /*
     * Deletes the record pointed to by tid and updates the page header accordingly.
     */
    bool SPSegment::remove(TID tid);

    /*
     * Returns the read-only record (cf. Record.hpp on the website) associated with TID tid.
     */
    Record SPSegment::lookup(TID tid);

    /*
     * Updates the record pointed to by tid with the content of record r.
     */
    bool SPSegment::update(TID tid, const Record& r);

};


#endif // SPSEGMENT_HPP
