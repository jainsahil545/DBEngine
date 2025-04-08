#include "bufferpool.h"

BufferPool::BufferPool(int poolSize, DiskManager *diskManager)
    : poolSize_(poolSize), diskManager_(diskManager)
{
    frames_.resize(poolSize_);
    for(auto &frame : frames_) {
        frame.pinCount = 0;
        frame.isDirty = false;
        // Mark an empty frame by setting page id to -1.
        frame.page.setPageId(-1);
        frame.lastAccessTime = std::chrono::steady_clock::now();
    }
    pageTable_.clear();
}

BufferPool::~BufferPool() {
    flushAllPages();
}

int BufferPool::findVictim() {
    // Use min_element to find the unpinned frame with the oldest access time.
    auto victimIt = std::min_element(frames_.begin(), frames_.end(),
        [](const Frame &a, const Frame &b) {
            // Only consider unpinned frames. If one is pinned, it cannot be evicted.
            if (a.pinCount == 0 && b.pinCount != 0) return true;
            if (a.pinCount != 0 && b.pinCount == 0) return false;
            return a.lastAccessTime < b.lastAccessTime;
        });
    // Ensure the victim is unpinned.
    if (victimIt != frames_.end() && victimIt->pinCount == 0)
        return std::distance(frames_.begin(), victimIt);
    return -1;
}

bool BufferPool::loadPageIntoFrame(int frameIndex, int pageId) {
    // Read the page from disk into the given frame.
    if (!diskManager_->readPage(pageId, frames_[frameIndex].page))
        return false;
    frames_[frameIndex].pinCount = 1;
    frames_[frameIndex].lastAccessTime = std::chrono::steady_clock::now();
    // Set the page's id (if not already set).
    frames_[frameIndex].page.setPageId(pageId);
    return true;
}

Page* BufferPool::fixPage(int pageId, bool isWrite) {
    // Check if the page is already in cache.
    auto it = pageTable_.find(pageId);
    if (it != pageTable_.end()) {
        int index = it->second;
        frames_[index].pinCount++;
        frames_[index].lastAccessTime = std::chrono::steady_clock::now();
        if (isWrite)
            frames_[index].isDirty = true;
        return &(frames_[index].page);
    }

    // Try to find an empty frame (unassigned frame).
    auto emptyIt = std::find_if(frames_.begin(), frames_.end(), [](const Frame &frame) {
        return (frame.pinCount == 0 && frame.page.getPageId() == -1);
    });

    if (emptyIt != frames_.end()) {
        int index = std::distance(frames_.begin(), emptyIt);
        if (!diskManager_->readPage(pageId, frames_[index].page))
            return nullptr;
        frames_[index].pinCount = 1;
        frames_[index].lastAccessTime = std::chrono::steady_clock::now();
        frames_[index].isDirty = isWrite;  // Mark dirty only if it's a write request.
        frames_[index].page.setPageId(pageId);
        pageTable_[pageId] = index;
        return &(frames_[index].page);
    } else {
        // No empty frame available, so select a victim based on LRU.
        int index = findVictim();
        if (index == -1)
            return nullptr; // All frames are pinned.
        
        // Before reusing, write back the victim if it is dirty.
        int victimPageId = frames_[index].page.getPageId();
        if (frames_[index].isDirty && victimPageId != -1) {
            diskManager_->writePage(victimPageId, frames_[index].page);
        }
        // Remove the victim's entry from the page table.
        pageTable_.erase(victimPageId);
        
        // Load the new page into the victim frame.
        if (!diskManager_->readPage(pageId, frames_[index].page))
            return nullptr;
        frames_[index].pinCount = 1;
        frames_[index].lastAccessTime = std::chrono::steady_clock::now();
        frames_[index].isDirty = isWrite;
        frames_[index].page.setPageId(pageId);
        pageTable_[pageId] = index;
        return &(frames_[index].page);
    }
}

void BufferPool::unfixPage(Page* page, bool isDirty) {
    int pageId = page->getPageId();
    auto it = pageTable_.find(pageId);
    if (it != pageTable_.end()) {
        int index = it->second;
        if (frames_[index].pinCount > 0)
            frames_[index].pinCount--;
        if (isDirty)
            frames_[index].isDirty = true;
        frames_[index].lastAccessTime = std::chrono::steady_clock::now();
    }
}

void BufferPool::flushAllPages() {
    for (auto &frame : frames_) {
        if (frame.isDirty && frame.pinCount == 0 && frame.page.getPageId() != -1) {
            diskManager_->writePage(frame.page.getPageId(), frame.page);
            frame.isDirty = false;
        }
    }
}
