#ifndef BTREE_HPP
#define BTREE_HPP

#include "../buffer/BufferFrame.hpp"
#include "../buffer/BufferManager.hpp"
#include "../buffer/PID.cpp"
#include "../segment/Segment.cpp"
#include "../segment/TID.cpp"
#include <string.h> // memcpy
#include <algorithm> // lower_bound
#include <iostream>


template<class K, class CMP = std::less<K> >
class BTree : public Segment {
    class Node {
    protected:
        bool leaf;
        size_t count;

        Node(bool leaf) : leaf(leaf), count(0) { };

    public:
        inline bool isLeaf() {
            return leaf;
        }

        virtual bool isFull() = 0;

        virtual K maximumKey() = 0;

        virtual K minimumKey() = 0;

        virtual K split(BufferFrame *bufferFrame) = 0;
    };

    class InnerNode : public Node {
        /**
         * Compute the order of a node given the page, key and page id size.
         */
        static const size_t order =
                (BufferFrame::frameSize - sizeof(Node) - sizeof(uint64_t)) /
                (sizeof(K) + sizeof(uint64_t));

        /*
         * Store the keys and child "pointer" in separate arrays.
         */
        K keys[order];
        uint64_t children[order + 1];

        InnerNode() : Node(false) { };

    public:
        InnerNode(uint64_t firstChild, uint64_t secondChild, K separator) : Node(false) {
            keys[0] = separator;
            children[0] = firstChild;
            children[1] = secondChild;
            this->count = 2;
        };

        bool isFull() {
            return this->count == order + 1;
        }

        K maximumKey() {
            return keys[this->count - 2];
        }

        K minimumKey() {
            return keys[0];
        }

        /**
         * Get the position of the key in the array with binary search.
         */
        size_t getKeyIndex(K key) {
            return std::lower_bound(keys, keys + this->count - 1, key, cmp) - keys;
        }

        /**
         * Get the child (i.e. the id of the page) of a given key.
         */
        uint64_t getChild(K key) {
            return children[getKeyIndex(key)];
        }

        /**
         * Insert an element into an inner node.
         * We simply overwrite the existing value if the key already exists.
         */
        void insert(K key, uint64_t child) {
            auto index = getKeyIndex(key);

            if (index < this->count - 1) {
                if (!cmp(key, keys[index])) {
                    children[index] = child;
                    return;
                } else {
                    // move existing entries
                    memmove(keys + index + 1, keys + index, (this->count - index - 1) * sizeof(K));
                    memmove(children + index + 2, children + index + 1, (this->count - index - 1) * sizeof(uint64_t));
                }
            }

            // insert new entry
            keys[index] = key;
            children[index + 1] = child;
            this->count++;
        }

        /**
         * Remove a separator key from an inner node.
         */
        bool remove(K key) {
            auto index = getKeyIndex(key);
            if (index == this->count)
                return false;

            // move existing entries
            memmove(keys + index, keys + index + 1, (this->count - index - 1) * sizeof(K));
            memmove(children + index, children + index + 1, (this->count - index - 1) * sizeof(uint64_t));
            this->count--;
            return true;
        }

        /**
         * Split an inner node and return the separator key that should be inserted into the parent.
         */
        K split(BufferFrame *bufferFrame) {
            InnerNode *newInnerNode = new(bufferFrame->getData()) InnerNode();

            auto middle = this->count / 2;
            this->count -= middle;
            newInnerNode->count = middle;

            auto keysSecondHalf = keys + this->count;
            auto childrenSecondHalf = children + this->count;
            std::move(keysSecondHalf, keysSecondHalf + (middle - 1) * sizeof(K), newInnerNode->keys);
            std::move(childrenSecondHalf, childrenSecondHalf + middle * sizeof(uint64_t), newInnerNode->children);

            return keys[this->count - 1];
        }
    };

    class LeafNode : public Node {
        /**
         * Compute the order of a node given the page, key and TID size.
         */
        static const size_t order = (BufferFrame::frameSize - sizeof(Node)) /
                                    (sizeof(K) + sizeof(TID));

        /**
         * Store keys and TIDs in separate arrays.
         */
        K keys[order];
        TID tids[order];

    public:
        LeafNode() : Node(true) { };

        bool isFull() {
            return this->count == order;
        }

        K maximumKey() {
            return keys[this->count - 1];
        }

        K minimumKey() {
            return keys[0];
        }

        /**
         * Get the position of the key in the array with binary search.
         */
        size_t getKeyIndex(K key) {
            return std::lower_bound(keys, keys + this->count, key, cmp) - keys;
        }

        /**
         * Check if the key exists in the node, return true.
         * Otherwise, false.
         */
        bool getTID(K key, TID &tid) {
            auto index = getKeyIndex(key);

            if (index == this->count || cmp(key, keys[index])) {
                return false;
            }

            tid = tids[index];
            return true;
        }

        /**
         * Insert an entry into a node.
         * If the key already exists in our node, we overwrite the TID.
         */
        void insert(K key, TID tid) {
            auto index = getKeyIndex(key);
            if (index < this->count) {
                if (!cmp(key, keys[index])) { // existing key?
                    tids[index] = tid; // overwrite existing value
                    return;
                }
            }

            // move existing entries which are on the right of the new element
            memmove(keys + index + 1, keys + index, (this->count - index) * sizeof(K));
            memmove(tids + index + 1, tids + index, (this->count - index) * sizeof(TID));
            // insert new entry
            keys[index] = key;
            tids[index] = tid;
            this->count++;
        }

        /**
         * Remove an entry from a node.
         * Returns true if successful and false if the key was not found in the node.
         */
        bool remove(K key) {
            auto index = getKeyIndex(key);
            if (index == this->count)
                return false;

            // Move remaining entries which were on the left of the new element
            memmove(keys + index, keys + index + 1, (this->count - index - 1) * sizeof(K));
            memmove(tids + index, tids + index + 1, (this->count - index - 1) * sizeof(TID));
            this->count--;
            return true;
        }

        /*
         * Split the leaf node stored in the given buffer frame into 2.
         * Return the separator key which will be the maximum key of the left leaf after the separation.
         */
        K split(BufferFrame *bufferFrame) {
            // New node that will contain the right half of the old
            LeafNode *newLeaf = new(bufferFrame->getData()) LeafNode();

            auto middle = this->count / 2;
            this->count -= middle;
            newLeaf->count = middle;

            // Move keys and TIDs to the new leaf
            auto keysSecondHalf = keys + this->count;
            auto tidsSecondHalf = tids + this->count;
            std::move(keysSecondHalf, keysSecondHalf + middle, newLeaf->keys);
            std::move(tidsSecondHalf, tidsSecondHalf + middle, newLeaf->tids);

            return maximumKey();
        }
    };

public:
    /**
     * Instantiate a new BTree. Creates an empty leaf node and sets it as the root.
     */
    BTree(BufferManager &bufferManager, uint16_t segmentId) : Segment(bufferManager, segmentId), root(0), numberOfEntries(0) {
        BufferFrame *bufferFrame = bufferManager.fixPage(PID(segmentId, root), true);
        void *dataPointer = bufferFrame->getData();
        new(dataPointer) LeafNode();
        bufferManager.unfixPage(bufferFrame, true);
        size = 1;
    };

    
    size_t getNumberOfEntries() {
        return numberOfEntries;
    }

    /**
     * Lookup an entry in the B-Tree.
     * Returns true if the key was found, false otherwise.
     * The second parameter TID is given a reference to the found tuple.
     */
    bool lookup(K key, TID &tid) {
        BufferFrame *bufferFrame = findLeaf(key, false);
        LeafNode *leaf = static_cast<LeafNode *>(bufferFrame->getData());
        auto found = leaf->getTID(key, tid);
        bufferManager.unfixPage(bufferFrame, false);
        return found;
    }

    /**
     * Inserts a new entry into the btree.
     * For concurrent access we use the "safe" inner pages approach.
     *
     */
    void insert(K key, TID tid) {
        this->numberOfEntries++;

        // start with the root
        BufferFrame *bufferFrame = bufferManager.fixPage(PID(segmentId, root), true);
        bool bufferFrameIsDirty = false;
        Node *node = static_cast<Node *>(bufferFrame->getData());

        // store data about direct parent for concurrency implementation
        BufferFrame *bufferFrameOfParent = nullptr;
        bool bufferFrameOfParentIsDirty = false;

        while (true) {
            if (node->isFull()) {
                // we need to create a new node
                uint64_t newPageID = ++this->size;
                BufferFrame *newBufferFrame = bufferManager.fixPage(PID(segmentId, newPageID), true);

                K separator;
                if (node->isLeaf()) {
                    LeafNode *oldLeaf = reinterpret_cast<LeafNode *>(node);
                    separator = oldLeaf->split(newBufferFrame);
                } else {
                    InnerNode *oldInner = reinterpret_cast<InnerNode *>(node);
                    separator = oldInner->split(newBufferFrame);
                }

                // Is this the root?
                if (bufferFrameOfParent == nullptr) {
                    // create another page and move the old root there
                    uint64_t movedOldRootID = ++this->size;
                    BufferFrame *movedOldRootBufferFrame = bufferManager.fixPage(PID(segmentId, movedOldRootID), true);
                    memcpy(movedOldRootBufferFrame->getData(), bufferFrame->getData(), BufferFrame::frameSize);

                    // Create the new root with the old (moved) root as its first child
                    new(bufferFrame->getData()) InnerNode(movedOldRootID, newBufferFrame->getPageID().getPage(),
                                                          separator);

                    bufferFrameOfParent = bufferFrame;
                    bufferFrameOfParentIsDirty = true;

                    bufferFrame = movedOldRootBufferFrame;
                    bufferFrameIsDirty = true;

                } else {
                    // not the root, so just insert separator into the parent
                    InnerNode *parentNode = static_cast<InnerNode *>(bufferFrameOfParent->getData());

                    parentNode->insert(separator, newBufferFrame->getPageID().getPage());
                    bufferFrameIsDirty = true;
                    bufferFrameOfParentIsDirty = true;
                }

                // insert entry
                if (!cmp(separator, key)) {
                    bufferManager.unfixPage(newBufferFrame, true);
                    newBufferFrame = NULL;
                } else {
                    bufferManager.unfixPage(bufferFrame, true);
                    bufferFrame = newBufferFrame;
                }
                node = static_cast<Node *>(bufferFrame->getData());

            } else {
                if (node->isLeaf()) {
                    // found the correct leaf & we have enough space left
                    LeafNode *leaf = reinterpret_cast<LeafNode *>(node);
                    leaf->insert(key, tid);

                    if (bufferFrameOfParent != nullptr) {
                        bufferManager.unfixPage(bufferFrameOfParent, bufferFrameOfParentIsDirty);
                    }

                    bufferManager.unfixPage(bufferFrame, true);
                    return;
                } else {
                    // traverse without splitting
                    InnerNode *innerNode = reinterpret_cast<InnerNode *>(node);
                    uint64_t nextID = innerNode->getChild(key);

                    BufferFrame *bufferFrameOfChild = bufferManager.fixPage(PID(segmentId, nextID), true);
                    if (bufferFrameOfParent != nullptr) {
                        bufferManager.unfixPage(bufferFrameOfParent, bufferFrameOfParentIsDirty);
                    }

                    bufferFrameOfParent = bufferFrame;
                    bufferFrameOfParentIsDirty = bufferFrameIsDirty;

                    bufferFrame = bufferFrameOfChild;
                    bufferFrameIsDirty = false;

                    node = static_cast<Node *>(bufferFrame->getData());
                }
            }
        }
    }

    /**
     * Deletes an entry from a B-Tree.
     * We ignore underfull nodes/leafs.
     */
    bool erase(K key) {
        BufferFrame *bufferFrame = findLeaf(key, false);
        LeafNode *leaf = static_cast<LeafNode *>(bufferFrame->getData());

        auto found = leaf->remove(key);
        if (found)
            this->numberOfEntries--;

        bufferManager.unfixPage(bufferFrame, found);
        return found;
    };

protected:
    /**
     * Specified comparator, default value is less<K>.
     */
    static CMP cmp;

private:
    /**
     * Page ID of the root node.
     */
    uint64_t root;

    /**
     * Number of elements currently stored in the BTree.
     */
    size_t numberOfEntries;

    /**
     * Helper function to traverse down to the correct leaf given a key.
     * If lock is true, the page of the current node will be held exclusively.
     * The function returns a pointer to the bufferFrame of the leaf node.
     */
    BufferFrame *findLeaf(K &key, bool lock) {
        // Start with the root
        BufferFrame *bufferFrame = bufferManager.fixPage(PID(segmentId, root), lock);
        Node *node = static_cast<Node *>(bufferFrame->getData());

        // Traverse to the leaf
        while (!node->isLeaf()) {
            InnerNode *innerNode = reinterpret_cast<InnerNode *>(node);
            uint64_t nextID = innerNode->getChild(key);

            BufferFrame *bufferFrameOfChild = bufferManager.fixPage(PID(segmentId, nextID), true);

            bufferManager.unfixPage(bufferFrame, false);
            bufferFrame = bufferFrameOfChild;

            node = static_cast<Node *>(bufferFrame->getData());
        }

        return bufferFrame;
    }
};

#endif //BTREE_HPP
