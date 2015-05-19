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
	auto bufferSize = sizeof(uint16_t) + getLen();
	char* buffer = new char[bufferSize];
	memcpy(buffer, &len, sizeof(uint16_t));
	memcpy(buffer+sizeof(uint16_t), getData(), getLen());
	return buffer;
}

uint16_t Record::serializedSize() const {
	return sizeof(uint16_t) + getLen();
}

Record* Record::deserialize(char* serialized) {
	uint16_t len = *((uint16_t*) serialized);
    return new Record(len, serialized + sizeof(uint16_t));
}