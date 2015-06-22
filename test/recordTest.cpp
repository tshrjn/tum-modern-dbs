#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <string.h>
#include <unordered_map>

#include "buffer/BufferManager.hpp"
#include "segment/Record.hpp"
#include "segment/SlottedPage.hpp"
#include "segment/TID.cpp"

using namespace std;

int main(int argc, char* argv[]) {
	const char* testStr = "Lorem ipsum blablablabla 42 random string";
	auto len = strlen(testStr);
	Record r_pre(len, testStr);
	char* serialized = r_pre.serialize();
	auto r_post = Record::deserialize(serialized);
	assert(r_pre.getLen() == r_post->getLen());
	assert(strcmp(testStr,r_post->getData()) == 0);
}