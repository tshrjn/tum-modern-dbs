#ifndef PID_CPP
#define PID_CPP

#include <stdlib.h>
#include <functional>
#include <math.h>
#include <sstream>

struct PID {

private:
    uint64_t value;
 
    static const uint64_t pageMask = 281474976710655L; // (pow(2, 48) - 1);

public:
    bool operator== (const PID& other) const {
        return value == other.value;
    }

    PID(uint16_t segment, uint64_t page) : value((uint64_t(segment) << 48) + page) { }
    PID(uint64_t value) :value(value) { }

    uint64_t getValue() const {
      return value;
    }

    uint16_t getSegment() const {
      return value >> 48;
    }

    uint64_t getPage() const {
      return value & pageMask;
    }

    operator std::string() const {
      std::stringstream ss;
      ss << "(" << getSegment() << "," << getPage() << ")";
      return ss.str();
    }
};

namespace std {
  template <>
  struct hash<PID>
  {
    std::size_t operator()(const PID &tid) const
    {
		return std::hash<uint64_t>()(tid.getValue());
    }
  };
}

#endif // PID_CPP