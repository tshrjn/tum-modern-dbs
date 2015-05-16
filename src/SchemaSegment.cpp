#include "SchemaSegment.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include "Types.hpp"


SchemaSegment::SchemaSegment(BufferManager& bufferManager, uint64_t id, std::unique_ptr<Schema> schema)
        : Segment(bufferManager, id), schema(std::move(schema))
{
    serialize();
}

SchemaSegment::SchemaSegment(BufferManager& bm, uint64_t id)
        : Segment(bm, id), schema(std::unique_ptr<Schema>(new Schema()))
{
    deserialize();
}

/**
 * @TODO: This only works if the schema requires less than a page in size.
 */
void SchemaSegment::serialize()
{
    size = 1;
    BufferFrame& bf = bufferManager.fixPage(id, true);
    void* dataPtr = bf.getData();

    std::string schemaString = schema->serialize();

    memmove( dataPtr, schemaString.c_str(), strlen(schemaString.c_str()) );

    // Persist to disk
    bf.flush();
    bufferManager.unfixPage(bf, true);
}

void SchemaSegment::deserialize()
{
    size = 1;
    BufferFrame& bf = bufferManager.fixPage(id, true);
    void* dataPtr = bf.getData();

    const char *data = static_cast<char *>(dataPtr);
  
    schema = Schema::deserialize(data);
    
    bf.flush();
    bufferManager.unfixPage(bf, false);
}