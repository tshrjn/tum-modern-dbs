#ifndef TID_CPP
#define TID_CPP

#include <stdlib.h>
#include <functional>

struct TID {
    //uint48_t pageID;
    uint32_t pageID;
    uint16_t slotID;

    bool operator== (const TID& other) const {
        return pageID == other.pageID && slotID == other.slotID;
    }
};


namespace std {
  template <>
  struct hash<TID>
  {
    std::size_t operator()(const TID &val) const
    {
		uint64_t c = val.pageID;
		c = c << 4;
		c += val.slotID;
		return std::hash<uint64_t>()(c);
    }
  };
}

#endif // TID_CPP