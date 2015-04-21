#ifndef BUFFERMANAGER_HPP
#define BUFFERMANAGER_HPP

#include "BufferFrame.hpp"

class BufferManager
{
public:

    BufferManager(unsigned pageCount);
    ~BufferManager();

    BufferFrame& fixPage(uint64_t pageId, bool exclusive);
    void unfixPage(BufferFrame& frame, bool isDirty);

private:

};

#endif //  BUFFERMANAGER_HPP
