#include "segment/SPSegment.hpp"


TID SPSegment::insert(const Record& r) {
	bufferManager.fixPage(PID(segmentId,0), true);
}

bool SPSegment::remove(TID tid)
{

}

Record SPSegment::lookup(TID tid)
{

}

bool SPSegment::update(TID tid, const Record& r)
{

}