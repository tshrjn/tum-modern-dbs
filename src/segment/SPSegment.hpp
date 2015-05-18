#ifndef SPSEGMENT_HPP
#define SPSEGMENT_HPP

#include "segment/Record.hpp"
#include "segment/TID.cpp"
#include "segment/Segment.cpp"

class SPSegment : Segment {
private:

public:
    /*
     * Constructor is given a reference to the buffer manager and a segment id.
     */
    SPSegment(BufferManager& bm, uint64_t segmentId) : Segment(bm, segmentId) {};

    /*
     * Searches through the segment's pages looking for a page with enough space to store r.
     * Returns the TID identifying the location where r was stored.
     *
     * Note: This can be implemented much more efficiently with a free space bitmap
     * as described in chapter 3, slide 3, but you are not required to do this.
     */
    TID insert(const Record& r);

    /*
     * Deletes the record pointed to by tid and updates the page header accordingly.
     */
    bool remove(TID tid);

    /*
     * Returns the read-only record (cf. Record.hpp on the website) associated with TID tid.
     */
    Record lookup(TID tid);

    /*
     * Updates the record pointed to by tid with the content of record r.
     */
    bool update(TID tid, const Record& r);

};


#endif // SPSEGMENT_HPP
