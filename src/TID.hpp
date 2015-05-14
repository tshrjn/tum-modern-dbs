#ifndef TID_HPP
#define TID_HPP

struct TID {
    uint48_t pageID;
    uint16_t slotID;

    bool operator== (const TID& other) const {
        return pageID == other.pageID && slotID == other.slotID;
    }
};

#endif // TID_HPP