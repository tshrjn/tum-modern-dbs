#include <cassert>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "schema/Types.cpp"
#include "buffer/BufferManager.hpp"
#include "schema/Schema.hpp"
#include "segment/SPSegment.hpp"
#include "operator/Register.hpp"
#include "operator/HashJoin.hpp"
#include "operator/Printer.hpp"
#include "operator/Projection.hpp"
#include "operator/Selection.hpp"
#include "operator/TableScan.hpp"

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
    auto recordSize = 2 * sizeof(int64_t) + 50;
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
    TableScan tableScan(relation, spSegment, bufferManager);
    tableScan.open();
    auto j = 0, count = 0;
    while (tableScan.next()) {
        std::vector<const Register *> registers = tableScan.getOutput();
        assert(registers.size() == 3);
        assert(registers[0]->getType() == Types::Tag::Integer);
        std::cout << registers[0]->getInt() << " = " << j << std::endl;
        assert(registers[0]->getInt() == j);
        assert(registers[1]->getType() == Types::Tag::Integer);
        std::cout << registers[1]->getInt() << " = " << 2 * recordCount - j << std::endl;
        assert(registers[1]->getInt() == 2 * recordCount - j);
        assert(registers[2]->getType() == Types::Tag::Char);
        std::cout << registers[2]->getString() << " = " << names[j % names.size()] << std::endl;
        assert(registers[2]->getString().compare(names[j % names.size()]) == 0);
        j++;
    }
    assert(j == recordCount);
    tableScan.close();

    std::cout << "TEST SUCCESSFUL!" << std::endl;
    return 0;
}