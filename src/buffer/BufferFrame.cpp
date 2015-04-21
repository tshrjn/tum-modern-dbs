#include "BufferFrame.hpp"

BufferFrame::BufferFrame() 
{
	state = FrameState::empty;
}

BufferFrame::~BufferFrame()
{

}


void* BufferFrame::getData()
{
	return data;
}

