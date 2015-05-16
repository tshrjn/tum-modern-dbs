CC = clang++
CXXFLAGS = -std=c++11 -march=native -O3 -Wall -pthread -g

BUFFER_O = src/buffer/BufferManager.cpp src/buffer/BufferFrame.cpp

all: sort buffer parser schema

sort: test/sort.cpp src/sort/externalSort.hpp
	$(CC) $(CXXFLAGS) -o bin/sort test/sort.cpp src/sort/externalSort.cpp 
	
buffer: test/buffertest.cpp $(BUFFER_O)
	$(CC) $(CXXFLAGS) -o bin/buffertest test/buffertest.cpp $(BUFFER_O)

Schema.o: src/schema/Schema.cpp src/schema/Schema.hpp
	$(CXX) $(CXXFLAGS) -c src/schema/Schema.cpp -o obj/Schema.o

Parser.o: src/schema/Parser.cpp src/schema/Parser.hpp
	$(CXX) $(CXXFLAGS) -c src/schema/Parser.cpp -o obj/Parser.o

parser: Schema.o Parser.o src/schema/Schema.hpp src/schema/Types.hpp src/schema/ParserCLI.cpp
	$(CXX) $(CXXFLAGS) obj/Parser.o obj/Schema.o src/schema/ParserCLI.cpp -o bin/parser

schema: test/schemaTest.cpp src/schema/Parser.cpp src/schema/Schema.cpp src/SchemaSegment.cpp $(BUFFER_O)
	$(CC) $(CXXFLAGS) -o bin/schema test/schemaTest.cpp src/schema/Parser.cpp src/schema/Schema.cpp src/SchemaSegment.cpp $(BUFFER_O)

slottedPages: test/slottedTest.cpp src/schema/Schema.cpp src/SPSegment.cpp $(BUFFER_O)
	$(CC) $(CXXFLAGS) -o bin/slottedPages test/slottedTest.cpp src/SPSegment.cpp $(BUFFER_O)


clean:
	rm -rf bin/* obj/*
