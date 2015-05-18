#ifndef H_Record_HPP
#define H_Record_HPP

#include <cstring>
#include <cstdlib>

// A simple Record implementation
class Record {
   uint16_t len;
   char* data;

public:
   // Assignment Operator: deleted
   Record& operator=(Record& rhs) = delete;
   // Copy Constructor: deleted
   Record(Record& t) = delete;
   // Move Constructor
   Record(Record&& t);
   // Constructor
   Record(unsigned len, const char* const ptr);
   // Destructor
   ~Record();
   // Get pointer to data
   const char* getData() const;
   // Get data size in bytes
   unsigned getLen() const;
   // Get total record length
   unsigned getTotalLen() const { return sizeof(this) - sizeof(char*) + getLen(); }
};

#endif