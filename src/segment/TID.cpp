#ifndef TID_CPP
#define TID_CPP

#include <stdlib.h>
#include <functional>
#include <math.h>
#include <sstream>


struct TID {

private:
    uint64_t value;

    // Bitmask to get the lower 16 bits
    static const uint64_t slotMask = 65535L; // pow(2, 16) - 1;

public:
    // Compare two TIDs
    bool operator== (const TID& other) const {
        return value == other.value;
    }

    // The value of the TID is the page shifted by 16 bits plus the slot
    // Therefore the lower 16 bits are the slot && the rest is the page
    TID(uint64_t page, uint16_t slot) : value((page << 16) + slot) { }

    TID() : value(0) {}
    TID(uint64_t tid) : value(tid) {}

    uint64_t getValue() const {
      return value;
    }

    // Shift the number 16 bits to right to get the value
    uint64_t getPage() const {
      return value >> 16;
    }

    // Just use the bitmask to get the slot
    uint16_t getSlot() const {
      return value & slotMask;
    }

    operator std::string() const {
      std::stringstream ss;
      ss << "(" << getPage() << "," << getSlot() << ")";
      return ss.str();
    }
};

// We can use the TID value as hash (it uniquely identifies the TID)
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