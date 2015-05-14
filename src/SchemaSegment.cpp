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

    std::vector<Schema::Relation> &relations = schema->relations;
    std::stringstream out;

    // Transform schema object to an easily readable string.
    out << relations.size() << std::endl;
    for (const Schema::Relation& rel : relations) {
        out << rel.name << " " << rel.attributes.size() << " " << rel.primaryKey.size() << std::endl;
        for (unsigned keyId : rel.primaryKey)
            out << keyId << " ";
        out << std::endl;
        for (const auto& attr : rel.attributes) {
            out << attr.name << " ";
            if (attr.type == Types::Tag::Integer) {
                out << "0";
            } else {
                out << "1";
            }
            out << " " << attr.len << " " << (attr.notNull ? "1" : "0") << std::endl;
        }
    }

    // Store the string of the schema in the data pointer of a buffer frame.
    std::string schemaString = out.str();
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

    const char *token = static_cast<char *>(dataPtr);
    std::stringstream ss;
    ss << token;

    // Build a new schema from scratch
    int relations;
    ss >> relations;
    // parse relations
    for (int i = 0; i < relations; i++) {
        std::string name;
        int attributes, keys;
        ss >> name >> attributes >> keys;

        Schema::Relation relation(name);

        // parse keys
        for (int j = 0; j < keys; j++) {
            unsigned key;
            ss >> key;
            relation.primaryKey.push_back(key);
        }

        // parse attributes
        for (int j = 0; j < attributes; j++) {
            std::string name;
            unsigned type;
            unsigned len;
            bool notNull;

            ss >> name >> type >> len >> notNull;

            Schema::Relation::Attribute attribute;
            attribute.name = name;
            if (type == 0) {
                attribute.type = Types::Tag::Integer;
            } else {
                attribute.type = Types::Tag::Char;
            }
            attribute.len = len;
            attribute.notNull = notNull;

            relation.attributes.push_back(attribute);
        }
        schema->relations.push_back(relation);
    }

    bf.flush();
    bufferManager.unfixPage(bf, false);
}