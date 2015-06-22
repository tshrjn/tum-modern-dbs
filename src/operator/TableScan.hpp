#ifndef TABLESCAN_HPP
#define TABLESCAN_HPP

#include <vector>

#include "operator/Operator.hpp"
#include "buffer/BufferManager.hpp"
#include "schema/Schema.hpp"
#include "segment/SPSegment.hpp"
#include "segment/SlottedPage.hpp"


class TableScan : public Operator {
    SPSegment &spSegment;
    BufferManager &bufferManager;
    BufferFrame *bufferFrame;


    uint64_t currentPageID;
    uint64_t lastID;

    SlottedPage *currentSlottedPage;
    uint16_t currentSlotID;

    std::vector<const Register *> registers;
    std::vector<Schema::Relation::Attribute> attributes;

public:
    TableScan(Schema::Relation &rel, SPSegment &spSegment, BufferManager &bufferManager);

    void open();

    bool next();

    std::vector<const Register *> getOutput() const;

    void close();
};


#endif //TABLESCAN_HPP
