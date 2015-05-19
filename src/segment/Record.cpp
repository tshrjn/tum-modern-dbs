#include "segment/Record.hpp"

Record::Record(Record&& t) : len(t.len), data(t.data) {
   t.data = nullptr;
   t.len = 0;
}

Record::Record(uint64_t len, const char* const ptr) : len(len) {
   data = static_cast<char*>(malloc(len));
   if (data)
      memcpy(data, ptr, len);
}

const char* Record::getData() const {
   return data;
}

uint64_t Record::getLen() const {
   return len;
}

Record::~Record() {
   free(data);
}

char* Record::serialize() const {
	auto bufferSize = sizeof(uint64_t) + getLen();
	char* buffer = new char[bufferSize];
	memcpy(buffer, &len, sizeof(uint64_t));
	memcpy(buffer+sizeof(uint64_t), getData(), getLen());
	return buffer;
}

uint64_t Record::serializedSize() const {
	return sizeof(uint64_t) + getLen();
}

Record* Record::deserialize(char* serialized) {
	uint64_t len = *((uint64_t*) serialized);
    return new Record(len, serialized + sizeof(uint64_t));
}