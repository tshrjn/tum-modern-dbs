#ifndef TID_CPP
#define TID_CPP

#include <stdlib.h>
#include <functional>

struct TID {
    uint32_t page;
    char __padding[2];
    uint16_t slot;

    bool operator== (const TID& other) const {
        return page == other.page && slot == other.slot;
    }

    TID(uint32_t page, uint16_t slot) : page(page), slot(slot) {}
};


namespace std {
  template <>
  struct hash<TID>
  {
    std::size_t operator()(const TID &val) const
    {
		uint64_t c = val.page;
		c = c << 16;
		c += val.slot;
		return std::hash<uint64_t>()(c);
    }
  };
}

#endif // TID_CPP