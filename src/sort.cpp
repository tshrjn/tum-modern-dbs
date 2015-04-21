#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "externalSort.hpp"

using namespace std;

bool isAscending (int fdOutput, uint64_t size) {
	// Nothing to check
	if (size <= 1) return true;

	if (lseek(fdOutput, 0, SEEK_SET) < 0) {
		cerr << "Rewinding of output file failed!"<< endl;
		return false;
	}
	
	size_t count = 0;
	uint64_t previousItem = 0;
	uint64_t currentItem = 0;

	count += read(fdOutput, &currentItem, sizeof(uint64_t));
	while (count < size) {
		previousItem = currentItem;
		count += read(fdOutput, &currentItem, sizeof(uint64_t));
		if (previousItem > currentItem)
			return false;
	}
	return true;
}

int main (int argc, char** argv) {
	if (argc != 4) {
		cerr << "Wrong number of parameters!" << endl;
		exit(EXIT_FAILURE);
	}
  	
  	int fdInput = open(argv[1], O_RDONLY);
  	int fdOutput = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0666);
  	uint64_t memSize = atoi(argv[3]) * 1024 * 1024;

	struct stat fileStats;
	stat(argv[1], &fileStats);
	uint64_t size = (uint64_t) fileStats.st_size / sizeof(uint64_t);

	externalSort(fdInput, size, fdOutput, memSize);

	if (isAscending(fdOutput, size))
		cout << "Success! Input was sorted correctly!" << endl;
	else
		cout << "Error! Something went wrong, it's not sorted!" << endl;
		
	close(fdInput);
	close(fdOutput);
	
	return 0;
}
