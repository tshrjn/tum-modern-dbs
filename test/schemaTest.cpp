#include <iostream>
#include <memory>
#include <string>
#include "buffer/BufferManager.hpp"
#include "schema/Parser.hpp"
#include "schema/Schema.hpp"
#include "segment/SchemaSegment.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <schema file>" << std::endl;
        return -1;
    }

    try {
        BufferManager bufferManager(10);

        // serialize schema to disk
        std::unique_ptr<Schema> schema = Parser(argv[1]).parse();
        SchemaSegment schemaSegment(bufferManager, 0, std::move(schema));
        // bufferManager.flushAll();
        // deserialize schema from disk
        SchemaSegment schemaSegment2(bufferManager, 0);
        // bufferManager.flushAll();

        // test if deserialized schema is equal to the original schema
        std::unique_ptr<Schema> originalSchema = Parser(argv[1]).parse();
        cout << originalSchema->toString() << endl;

        if (originalSchema->toString().compare(schemaSegment2.getSchema()->toString()) == 0) {
            cout << "Test successfull" << endl;
            return 0;
        }
    } catch (ParserError& pe) {
        cerr << "Parser Error: " << pe.what() << endl;
    } catch (exception& e) {
        cerr << e.what() << endl;
    }

    cerr << "Test failed" << endl;
    return 1;
}