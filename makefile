CC = clang++
CFLAGS = -std=c++11 -march=native -O3 -Wall -pthread

BUFFER_O = src/buffer/BufferManager.cpp src/buffer/BufferFrame.cpp

all: sort buffer

sort: test/sort.cpp src/externalSort.hpp
	$(CC) $(CFLAGS) -o bin/sort test/sort.cpp src/externalSort.cpp 
	
buffer: test/buffertest.cpp $(BUFFER_O)
	$(CC) $(CFLAGS) -o bin/buffertest test/buffertest.cpp $(BUFFER_O)



clean:
	rm -rf bin/*
