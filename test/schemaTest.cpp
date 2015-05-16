#include <iostream>
#include <memory>
#include <string>
#include "../src/buffer/BufferManager.hpp"
#include "../src/schema/Parser.hpp"
#include "../src/schema/Schema.hpp"
#include "../src/SchemaSegment.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <schema file>" << std::endl;
        return -1;
    }

    Parser parser(argv[1]);
    try {
        BufferManager bufferManager(1);

        // serialize schema to disk
        std::unique_ptr<Schema> schema = parser.parse();
        SchemaSegment schemaSegment(bufferManager, 0, std::move(schema));
        // deserialize schema from disk
        SchemaSegment schemaSegment2(bufferManager, 0);

        // test if deserialized schema is equal to the original schema
        std::unique_ptr<Schema> originalSchema = parser.parse();
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