#include <string>
#include <cstdint>
#include <cassert>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include "../src/buffer/BufferManager.hpp"
#include "../src/index/BTree.hpp"

/* Comparator functor for uint64_t*/
struct MyCustomUInt64Cmp {
    bool operator()(uint64_t a, uint64_t b) const {
        return a < b;
    }
};

template<unsigned len>
struct Char {
    char data[len];
};

/* Comparator functor for char */
template<unsigned len>
struct MyCustomCharCmp {
    bool operator()(const Char<len> &a, const Char<len> &b) const {
        return memcmp(a.data, b.data, len) < 0;
    }
};

typedef std::pair <uint32_t, uint32_t> IntPair;

/* Comparator for IntPair */
struct MyCustomIntPairCmp {
    bool operator()(const IntPair &a, const IntPair &b) const {
        if (a.first < b.first)
            return true;
        else
            return (a.first == b.first) && (a.second < b.second);
    }
};

template<class T>
const T &getKey(const uint64_t &i);

template<>
const uint64_t &getKey(const uint64_t &i) { return i; }

std::vector <std::string> char20;

template<>
const Char<20> &getKey(const uint64_t &i) {
    std::stringstream ss;
    ss << i;
    std::string s(ss.str());
    char20.push_back(std::string(20 - s.size(), '0') + s);
    return *reinterpret_cast<const Char<20> *>(char20.back().data());
}

std::vector <IntPair> intPairs;

template<>
const IntPair &getKey(const uint64_t &i) {
    intPairs.push_back(std::make_pair(i / 3, 3 - (i % 3)));
    return intPairs.back();
}


template<class T, class CMP>
void test(uint64_t n) {
    // Set up stuff, you probably have to change something here to match to your interfaces
    BufferManager bufferManager(100);
    BTree<T, CMP> bTree(bufferManager, 4);

    // Insert values
    for (uint64_t i = 0; i < n; ++i)
        bTree.insert(getKey<T>(i), TID(i*i));

    assert(bTree.getNumberOfEntries() == n);

    // Check if they can be retrieved
    for (uint64_t i = 0; i < n; ++i) {
        TID tid;
        assert(bTree.lookup(getKey<T>(i), tid));
        std::cout << tid.getValue() << "==" << TID(i*i).getValue() << std::endl;
        assert(tid == (TID(i*i)));
    }

    // Delete some values
    for (uint64_t i = 0; i < n; ++i)
        if ((i % 7) == 0)
            bTree.erase(getKey<T>(i));

    // Check if the right ones have been deleted
    for (uint64_t i = 0; i < n; ++i) {
        TID tid(i*i);
        if ((i % 7) == 0) {
            assert(!bTree.lookup(getKey<T>(i), tid));
        } else {
            assert(bTree.lookup(getKey<T>(i), tid));
            assert(tid.getValue() == i*i);
        }
    }

    // Delete everything
    for (uint64_t i = 0; i < n; ++i)
        bTree.erase(getKey<T>(i));

    assert(bTree.getNumberOfEntries() == 0);
}

int main(int argc, char *argv[]) {
    // Get command line argument
    const uint64_t n = (argc == 2) ? strtoul(argv[1], NULL, 10) : 100 * 1000ul;

    // Test index with 64bit unsigned integers
    test<uint64_t, MyCustomUInt64Cmp>(n);

    // Test index with 20 character strings
    test<Char<20>, MyCustomCharCmp<20>>(n);

    // Test index with compound key
    test<IntPair, MyCustomIntPairCmp>(n);

    std::cout << "test successful" << std::endl;

    return 0;
}
