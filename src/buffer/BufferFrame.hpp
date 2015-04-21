#ifndef BUFFERFRAME_HPP
#define BUFFERFRAME_HPP

#include <cstdint>
#include <cstddef>

const size_t blockSize = 4 * 4096;

enum FrameState {
	empty, // data not loaded yet
	clean,  // data loaded, unmodified
	dirty	// data loaded, has changed
};

class BufferFrame {
	private:
		uint64_t pageNumber;
		FrameState state;

		void* data;

	public:
		BufferFrame();
		~BufferFrame();

		void* getData();
};

#endif //  BUFFERFRAME_HPP
