#ifndef H_Record_HPP
#define H_Record_HPP

#include <cstring>
#include <cstdlib>
#include <stdint.h>

// A simple Record implementation
class Record {
   uint64_t len;
   char* data;

public:
   static const auto headerSize = sizeof(len);

   // Assignment Operator: deleted
   Record& operator=(Record& rhs) = delete;
   // Copy Constructor: deleted
   Record(Record& t) = delete;
   // Move Constructor
   Record(Record&& t);
   // Constructor
   Record(uint64_t len, const char* const ptr);
   // Destructor
   ~Record();
   // Get pointer to data
   const char* getData() const;
   // Get data size in bytes
   uint64_t getLen() const;

   // Get byte representation of record
   char* serialize() const;
   // Get record of bytes
   static Record* deserialize(char* data);
   // Get the buffer size for the serialized record
   uint64_t serializedSize() const;

};

#endif
