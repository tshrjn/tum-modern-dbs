#ifndef SPSEGMENT_HPP
#define SPSEGMENT_HPP

#include "segment/Record.hpp"
#include "segment/TID.cpp"
#include "segment/Segment.cpp"
#include "segment/SlottedPage.hpp"
#include <iostream>

class SPSegment : public Segment {
private:
    uint64_t numberPages = 0;

public:
    /*
     * Constructor is given a reference to the buffer manager and a segment id.
     */
    SPSegment(BufferManager& bm, uint16_t segmentId, uint64_t numberPages)
        : Segment(bm, segmentId), numberPages(numberPages) {};

    /*
     * Searches through the segment's pages looking for a page with enough space to store r.
     * Returns the TID identifying the location where r was stored.
     *
     * Note: This can be implemented much more efficiently with a free space bitmap
     * as described in chapter 3, slide 3, but you are not required to do this.
     */
    TID insert(Record* r);

    /*
     * Deletes the record pointed to by tid and updates the page header accordingly.
     */
   	bool remove(TID tid);

    /*
     * Returns the read-only record (cf. Record.hpp on the website) associated with TID tid.
     */
    Record* lookup(TID tid);

    /*
     * Updates the record pointed to by tid with the content of record r.
     */
    bool update(TID tid, Record* r);

};


#endif // SPSEGMENT_HPP
