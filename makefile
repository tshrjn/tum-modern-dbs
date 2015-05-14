CC = clang++
CXXFLAGS = -std=c++11 -march=native -O3 -Wall -pthread -g

BUFFER_O = src/buffer/BufferManager.cpp src/buffer/BufferFrame.cpp

all: sort buffer parser schema

sort: test/sort.cpp src/externalSort.hpp
	$(CC) $(CXXFLAGS) -o bin/sort test/sort.cpp src/externalSort.cpp 
	
buffer: test/buffertest.cpp $(BUFFER_O)
	$(CC) $(CXXFLAGS) -o bin/buffertest test/buffertest.cpp $(BUFFER_O)

Schema.o: src/Schema.cpp src/Schema.hpp
	$(CXX) $(CXXFLAGS) -c src/Schema.cpp -o obj/Schema.o

Parser.o: src/Parser.cpp src/Parser.hpp
	$(CXX) $(CXXFLAGS) -c src/Parser.cpp -o obj/Parser.o

parser: Schema.o Parser.o src/Schema.hpp src/Types.hpp src/parser.cpp
	$(CXX) $(CXXFLAGS) obj/Parser.o obj/Schema.o src/parser.cpp -o bin/parser

schema: test/schemaTest.cpp src/Parser.cpp src/Schema.cpp src/SchemaSegment.cpp $(BUFFER_O)
	$(CC) $(CXXFLAGS) -o bin/schema test/schemaTest.cpp src/Parser.cpp src/Schema.cpp src/SchemaSegment.cpp $(BUFFER_O)

slottedPages: test/slottedTest.cpp src/Schema.cpp src/SPSegment.cpp $(BUFFER_O)
	$(CC) $(CXXFLAGS) -o bin/slottedPages test/slottedTest.cpp src/SPSegment.cpp $(BUFFER_O)


clean:
	rm -rf bin/* obj/*
