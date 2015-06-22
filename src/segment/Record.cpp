#include "segment/Record.hpp"

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

char* Record::serialize() const {
	auto bufferSize = sizeof(unsigned) + getLen();
	char* buffer = new char[bufferSize];
	memcpy(buffer, &len, sizeof(unsigned));
	memcpy(buffer+sizeof(unsigned), getData(), getLen());
	return buffer;
}

unsigned Record::serializedSize() const {
	return sizeof(unsigned) + getLen();
}

Record* Record::deserialize(char* serialized) {
	unsigned len = *((unsigned*) serialized);
    return new Record(len, serialized + sizeof(unsigned));
}