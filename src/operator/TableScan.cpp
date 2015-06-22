#include "operator/TableScan.hpp"
#include "operator/Register.hpp"
#include <vector>
#include <cassert>

TableScan::TableScan(Schema::Relation &relation, SPSegment &spSegment, BufferManager &bufferManager) :
        spSegment(spSegment), bufferManager(bufferManager), bufferFrame(nullptr), attributes(relation.attributes) {
    registers.resize(attributes.size());
}

void TableScan::open() {
    assert(bufferFrame == nullptr);

    currentPageID = 0;
    lastID = spSegment.getSize() - 1;
}

bool TableScan::next() {
    // loop over pages
    while (true) {
        // load a new page if necessary
        if (bufferFrame == nullptr) {
            // check if we reached the end of the segment
            if (currentPageID > lastID)
                return false;

            bufferFrame = bufferManager.fixPage(PID(spSegment.getSegmentId(), currentPageID), false);
            currentSlottedPage = static_cast<SlottedPage*>(bufferFrame->getData());

            currentSlotID = 0;
            ++currentPageID;
        }

        // loop over the slots of the current page
        // TODO: GET NUMBER OF SLOTS : currentSlotID < slottedPage->getSlotCount()
        while (currentSlotID < 5) {
            auto slot = currentSlottedPage->getData(currentSlotID);
            ++currentSlotID;

            // TODO: HOW TO SKIP OVER FREE / INDIRECTION SLOTS

            // all types have fixed length
            char *recordPtr = slot;
            off_t recordOffset = 0;
            for (int i = 0; i < registers.size(); ++i) {
                Register *reg = new Register;
                reg->load(attributes[i].type, recordPtr + recordOffset);
                registers[i] = reg;
                recordOffset += attributes[i].len;
            }
            return true;
        }

        // reach end of page
        bufferManager.unfixPage(bufferFrame, false);
        bufferFrame = nullptr;
    }
}

std::vector<const Register *> TableScan::getOutput() const {
    return registers;
}

void TableScan::close() {
    if (bufferFrame != nullptr) {
        bufferManager.unfixPage(bufferFrame, false);
        bufferFrame = nullptr;
    }
}
