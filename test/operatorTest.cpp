#include <cassert>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "../src/schema/Types.cpp"
#include "../src/buffer/BufferManager.hpp"
#include "../src/schema/Schema.hpp"
#include "../src/segment/SPSegment.hpp"
#include "../src/operator/Register.hpp"
#include "../src/operator/HashJoin.hpp"
#include "../src/operator/Printer.hpp"
#include "../src/operator/Projection.hpp"
#include "../src/operator/Selection.hpp"
#include "../src/operator/TableScan.hpp"

const std::vector<std::string> names = {
        "Philip",
        "Andre",
        "Thomas",
        "Andrey",
        "Viktor",
        "Stupidly-Complicated-name"
};

void testRegister() {
    Register reg, reg2;

    std::cout << "Test Registers..." << std::endl;

    reg.setInt(42);
    assert(reg.getType() == Types::Tag::Integer);
    assert(reg.getInt() == 42);

    reg2.setString("Philip");
    assert(reg2.getType() == Types::Tag::Char);
    assert(reg2.getString().compare("Philip") == 0);

    std::cout << "test successful!" << std::endl;
}


int main(int argc, char *argv[]) {
    testRegister();

    std::cout << "Test operators..." << std::endl;
    std::cout << "Initialize test data" << std::endl;

    BufferManager bufferManager(100);
    SPSegment spSegment(bufferManager, 5, 1);
    Schema::Relation relation("Relation", 5, 1);

    // Create test Relation: |id|age|name
    Schema::Relation::Attribute attributeId;
    attributeId.name = "id";
    attributeId.type = Types::Tag::Integer;
    attributeId.len = sizeof(int64_t);
    relation.attributes.push_back(attributeId);

    Schema::Relation::Attribute attributeAge;
    attributeAge.name = "age";
    attributeAge.type = Types::Tag::Integer;
    attributeAge.len = sizeof(int64_t);
    relation.attributes.push_back(attributeAge);

    Schema::Relation::Attribute attributeName;
    attributeName.name = "name";
    attributeName.type = Types::Tag::Char;
    attributeName.len = 50;
    relation.attributes.push_back(attributeName);

    // Fill Relation with some values
    auto recordSize = 2 * sizeof(int64_t) + 32;
    auto recordCount = 10;
    for (int64_t i = 0; i < recordCount; ++i) {
        char data[recordSize];
        int64_t *intPtr = reinterpret_cast<int64_t *>(data);
        *intPtr = i; // id
        intPtr++;
        *intPtr = 2 * recordCount - i; // age
        intPtr++;
        char *namePtr = reinterpret_cast<char *>(intPtr);
        memset(namePtr, '\0', 50);
        strcpy(namePtr, names[i % names.size()].c_str()); // name
        Record *record = new Record(recordSize, data);
        spSegment.insert(record);
    }

    std::cout << "Test table scan..." << std::endl;
    // Test TableScan
    TableScan ts(relation, spSegment, bufferManager);
    ts.open();
    int64_t j = 0;
    while (ts.next()) {
        std::vector<const Register *> registers = ts.getOutput();
        assert(registers.size() == 3);
        assert(registers[0]->getType() == Types::Tag::Integer);
        assert(registers[0]->getInt() == j);
        assert(registers[1]->getType() == Types::Tag::Integer);
        assert(registers[1]->getInt() == 2 * recordCount - j);
        assert(registers[2]->getType() == Types::Tag::Char);
        assert(registers[2]->getString().compare(names[j % names.size()]) == 0);
        j++;
    }
    assert(j == recordCount);
    ts.close();

    return 0;
}