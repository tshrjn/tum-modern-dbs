#ifndef SCHEMASEGMENT_HPP
#define SCHEMASEGMENT_HPP
#include <memory>
#include "segment/Segment.cpp"
#include "buffer/BufferManager.hpp"
#include "buffer/PID.cpp"
#include "schema/Schema.hpp"

class SchemaSegment : public Segment {
public:
    /**
     * Create a schema segment with the given schema, serialize to disk.
     */
    SchemaSegment(BufferManager& bufferManager, uint16_t segmentId, std::unique_ptr<Schema> schema);

    /**
     * Deserialize schema segment from disk.
     */
    SchemaSegment(BufferManager& bufferManager, uint16_t segmentId);

    /*
     * Return reference of loaded schema.
     */
    std::unique_ptr<Schema> getSchema() {
        return std::move(schema);
    }

private:
    /**
     * Pointer to the database schema.
     */
    std::unique_ptr<Schema> schema;

    /**
     * Serialize schema to disk.
     */
    void serialize();

    /**
     * Deserialize from disk.
     */
    void deserialize();
};


#endif //SCHEMASEGMENT_HPP
