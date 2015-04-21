CC = clang++
CFLAGS = -std=c++11 -march=native -O3 -Wall -pthread
BUFFER_O = src/BufferManager.cpp src/BufferFrame.cpp

all: sort

sort: test/sort.cpp src/externalSort.hpp
	$(CC) $(CFLAGS) -o bin/sort test/sort.cpp src/externalSort.cpp 
	
clean:
	rm -rf bin/*
