#include "pages/Record.hpp"

Record::Record(Record&& t) : len(t.len), data(t.data) {
   t.data = nullptr;
   t.len = 0;
}

Record::Record(unsigned len, const char* const ptr) : len(len) {
   data = static_cast<char*>(malloc(len));
   if (data)
      memcpy(data, ptr, len);
}

const char* Record::getData() const {
   return data;
}

unsigned Record::getLen() const {
   return len;
}

Record::~Record() {
   free(data);
}