#include "segment/SchemaSegment.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include "schema/Types.cpp"
#include <unordered_set>
#include <math.h> 


SchemaSegment::SchemaSegment(BufferManager& bufferManager, uint64_t segmentId, std::unique_ptr<Schema> schema)
        : Segment(bufferManager, segmentId), schema(std::move(schema))
{
    serialize();
}

SchemaSegment::SchemaSegment(BufferManager& bm, uint64_t segmentId)
        : Segment(bm, segmentId), schema(std::unique_ptr<Schema>(new Schema()))
{
    deserialize();
}

void SchemaSegment::serialize()
{
    std::unordered_set<BufferFrame*> framePtrs;

    // fix first buffer frame to store the number of schema-pages
    // @TODO: would be nicer to store the number of schema-bytes and derive the number of pages
    BufferFrame* bf = bufferManager.fixPage(PID(segmentId,0), true);
    char *bfData = static_cast<char *>(bf->getData());
    framePtrs.insert(bf);

    // serialize the schema and get a pointer to a byte array
    std::string schemaString = schema->serialize();
    auto length = strlen(schemaString.c_str());
    auto data = schemaString.c_str();

    // calculate additional schema pages
    uint64_t additionalSchemaPages = 0;
    if(length > (BufferFrame::frameSize - sizeof(uint64_t))) {
        additionalSchemaPages = (uint64_t) ceil(
            (length - (BufferFrame::frameSize - sizeof(uint64_t)))
            * 1.0 / BufferFrame::frameSize);
    }

    std::cout << "Strlen of schema: " << length << std::endl;
    std::cout << "Additional pages: " << additionalSchemaPages << std::endl;

    // store the first buffer frame
    memcpy(bfData, &additionalSchemaPages, sizeof(uint64_t));
    memcpy(bfData + sizeof(uint64_t), data, BufferFrame::frameSize - sizeof(uint64_t));
    auto posBuffer = BufferFrame::frameSize - sizeof(uint64_t);

    // create additional buffer frames
    for(int i = 0; i < additionalSchemaPages; i++) {
        BufferFrame* nf = bufferManager.fixPage(PID(segmentId, i+1), true);
        char *nd = static_cast<char *>(nf->getData());
        memcpy(nd, data + posBuffer, BufferFrame::frameSize);
        posBuffer += BufferFrame::frameSize;
        framePtrs.insert(nf);
    }

    // unfix all pages
    for(const auto& framePtr: framePtrs) {
        bufferManager.unfixPage(framePtr, true);
    }
}

void SchemaSegment::deserialize()
{
    // first load the first page to get the number of pages that we need to read
    std::unordered_set<BufferFrame*> framePtrs;
    BufferFrame* bf = bufferManager.fixPage(PID(segmentId,0), false);
    framePtrs.insert(bf);

    // in the first uint64_t of page 0 we store the number of additional pages (excluding 0) that we need to load
    const char *data = static_cast<char *>(bf->getData());
    uint64_t additionalSchemaPages = static_cast<uint64_t> (*data);
    auto schemaLength = (additionalSchemaPages + 1) * BufferFrame::frameSize - sizeof(uint64_t); 
    char buffer[schemaLength];

    // copy the rest of the page to the buffer
    memcpy(buffer, data + sizeof(uint64_t), BufferFrame::frameSize - sizeof(uint64_t));
    auto posBuffer = BufferFrame::frameSize - sizeof(uint64_t);

    // iteratively copy all other data
    for(int i = 0; i < additionalSchemaPages; i++) {
        BufferFrame* nf = bufferManager.fixPage(PID(segmentId,i+1), false);
        const char *nd = static_cast<char *>(nf->getData());
        memcpy(buffer + posBuffer, nd, BufferFrame::frameSize);
        posBuffer += BufferFrame::frameSize;
        framePtrs.insert(nf);
    }

    // deserialize with the created buffer
    schema = Schema::deserialize(buffer);
    
    // unfix all pages
    for(const auto& framePtr: framePtrs) {
        bufferManager.unfixPage(framePtr, false);
    }
}