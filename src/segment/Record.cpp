#include "segment/Record.hpp"

Record::Record(Record&& t) : len(t.len), data(t.data) {
   t.data = nullptr;
   t.len = 0;
}

Record::Record(uint32_t len, const char* const ptr) : len(len) {
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
	auto bufferSize = sizeof(uint32_t) + getLen();
	char* buffer = new char[bufferSize];
	memcpy(buffer, &len, sizeof(uint32_t));
	memcpy(buffer+sizeof(uint32_t), getData(), getLen());
	return buffer;
}

uint16_t Record::serializedSize() const {
	return sizeof(uint32_t) + getLen();
}

Record* Record::deserialize(char* serialized) {
	uint32_t len = *((uint32_t*) serialized);
    return new Record(len, serialized + sizeof(uint32_t));
}