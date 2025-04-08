#pragma once
#include <vector>
#include <unordered_map>
#include "diskmanager.h"
#include "page.h"

class BufferPool {
public:
    BufferPool(int poolSize, DiskManager *diskManager);
    ~BufferPool();

    // Returns a pointer to the page if successfully fixed, or nullptr on error.
    Page* fixPage(int pageId, bool isWrite);

    // Unfixes (unpins) the page. Marks as dirty if needed.
    void unfixPage(Page* page, bool isDirty);

    // Writes all dirty pages in the pool back to disk.
    void flushAllPages();

private:
    struct Frame {
        Page page;
        int pinCount;
        bool isDirty;
        // Additional eviction-related data (e.g., last access time for LRU)
    };

    int poolSize_;
    DiskManager* diskManager_;
    std::vector<Frame> frames_;
    std::unordered_map<int, int> pageTable_; // Maps pageId to index in frames_

    // Finds a victim frame index based on the replacement policy.
    int findVictim();

    // (Optional) Helper function to load a page into a specific frame.
    bool loadPageIntoFrame(int frameIndex, int pageId);
};
