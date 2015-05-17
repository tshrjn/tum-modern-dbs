#ifndef TID_CPP
#define TID_CPP

#include <stdlib.h>
#include <functional>
#include <math.h>
#include <sstream>


struct TID {

private:
    uint64_t value;

    static const uint64_t slotMask = 65535L; // pow(2, 16) - 1;

public:
    bool operator== (const TID& other) const {
        return value == other.value;
    }

    TID(uint64_t page, uint16_t slot) : value((page << 16) + slot) { }

    uint64_t getValue() const {
      return value;
    }

    uint64_t getPage() const {
      return value >> 16;
    }

    uint16_t getSlot() const {
      return value & slotMask;
    }

    operator std::string() const {
      std::stringstream ss;
      ss << "(" << getPage() << "," << getSlot() << ")";
      return ss.str();
    }
};

namespace std {
  template <>
  struct hash<TID>
  {
    std::size_t operator()(const TID &tid) const
    {
		return std::hash<uint64_t>()(tid.getValue());
    }
  };
}

#endif // TID_CPP